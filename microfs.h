/* 
  MicroFS
  A low-footprint wear-leveling file system for Arduino/AVR microcontrollers
  
  features:
  - 2 byte on-disk overhead per file
  - 4 byte in-memory overhead per open file
  - (optional) wear-levelling
  
  limits:
  - 255 files (files are unnamed, can be identified using a number from 1 to 255 inclusive)
  - 0-255 bytes per file
  - no directories, ACLs, file[cma]time, etc.
  - files are allocated as contiguos chunks of EEPROM, file fragments are not allowed 
    (this implies that to allocate a file of size N bytes, a chunk of size N+2 must be free)
  - if power is lost (or execution halted) inside a CRITICAL SECTION, the whole fs is corrupted
  
  design:
  Each file on disk is represented by a contiguous chunk of bytes made up of a file header immediately 
  followed by the file data:
  
  +---- stride (file_length + 2) ----+
  |                                  |
  ILDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDILDDD...   <-- EEPROM contents
  |||                               |
  ||+-- file_length bytes of data --+
  |+- byte file_length 
  +- byte file_id 
  
  Files are placed one after another with no padding in between. This effectively yields an on-disk
  linked list: to get the position of the header of the following file, we just need to add the 
  stride to the position of the current file header.
  
  file_ids are required to be unique and in the range 1-255 inclusive: file_id 0 is used to mark 
  unallocated space and can occur multiple times

  TODO:
  - files can't be changed in size (extend in place, copy-and-extend)
  - free space defragmentation
  - check after write to ensure successful writes
*/

#ifndef MICROFS
#define MICROFS

#include <avr/eeprom.h>

static byte eeprom_read(size_t pos) {
  if (pos < 0 || pos > E2END)
    return 0;
  return eeprom_read_byte((uint8_t*)pos);
}

static bool eeprom_update(size_t pos, byte val) {
  if (pos < 0 || pos > E2END)
    return false;
  // the avr/eeprom.h for arduino appears to lack eeprom_update_byte
  if (eeprom_read(pos) == val)
    return true;
  eeprom_write_byte((uint8_t*)pos, val);
  return eeprom_read(pos) == val;
}

class microfsfile {
  
  friend class microfs;
  
  // IN-EEPROM MEMBERS //
  byte id; // id of the file, id 0 is reserved for unallocated space
  byte size; // size of file (bytes)
  
  // IN-MEMORY MEMBERS //
  size_t offset; // offset of file header in EEPROM

  // construct the "invalid" file
  microfsfile() {
    id = 0;
    size = 0;
    offset = -1;
  }
  
  // construct an unallocated file
  microfsfile(byte file_id, byte file_size) {
    id = file_id;
    size = file_size;
    offset = -1;
  }
  
  // construct an allocated file
  microfsfile(byte file_id, byte file_size, size_t eeprom_offset) {
    id = file_id;
    size = file_size;
    offset = eeprom_offset;
  }
  
  size_t stride() {
    return ((size_t)2) + ((size_t)size);
  }
  
  // true if this is an unallocated chunk that can be merged with one of its neighbors
  bool mergeable() {
    return is_valid() && (id == 0) && (size < 255);
  }  
  
  public:
  bool is_valid() {
    return offset >= 0; // FIXME  
  }
  
  byte get_size() {
    return size;
  }
  
  bool write_byte(byte pos, byte value) {
    if (!is_valid())
      return false;
    size_t off = offset + ((size_t)2) + ((size_t)pos);
    return eeprom_update(off, value);
  }
  
  byte read_byte(byte pos) {
    if (!is_valid())
      return 0;
    size_t off = offset + 2 + pos;
    return eeprom_read(off);
  }
  
  byte read_bytes(byte pos, byte* buf, byte len) {
    if (!is_valid())
      return 0;
    if (pos >= size)
      return 0;
    len = min(len, size-pos);
    for (byte i=0; i<len; i++) {
      buf[i] = read_byte(pos+i);
    }
    return len;
  }
  
};

class microfs {
  
  const size_t size;
  bool consistent;
  
  public:
  microfs() : size(E2END+1) {
  }
  
  void format() {
    size_t pos = 0;
    while (pos < size) {
      microfsfile unallocated(0, max(0, min(255, size-pos-2)));
      write_header(pos, unallocated);
      pos += unallocated.stride();
    }
  }
  
  bool check_disk() {
    size_t pos = 0;
    byte mask[256/8] = {0};
    while (pos < size) {
      microfsfile f = read_header(pos);
      // check that the file is not overflowing
      if (pos + f.stride() > size)
        return false;
      // check for duplicate file ids
      byte used = mask[f.id/8] & (((byte)1) << (f.id%8));
      if (f.id != 0 && used)
        return false;
      mask[f.id/8] |= (((byte)1) << (f.id%8));
      pos += f.stride();
    }
    return true;
  }
  
  size_t used() {
    size_t pos = 0, used = 0;
    while (pos < size) {
      microfsfile f = read_header(pos);
      if (f.is_valid() && f.id != 0) {
        used += f.size;
      }
      pos += f.stride();
    }
    return used;
  }
  
  microfsfile open(byte file_id) {
    size_t pos = 0;
    while (pos < size) {
      microfsfile f = read_header(pos);
      if (f.is_valid() && f.id == file_id) {
        return f;
      }
      pos += f.stride();
    }
    return microfsfile();
  }
  
  // allocate and create a new file on disk of size <size>
  microfsfile create(byte size) {
    microfsfile unallocated = find_alloc(size);
    if (!unallocated.is_valid())
      return microfsfile();
    byte id = find_id();
    if (id == 0)
      return microfsfile();
    microfsfile newfile(id, size);
    size_t newfile_pos = unallocated.offset;
    // find_alloc will return either a chunk of size == size or size >= size + 2
    // in the second case, we write a new unallocated header at the end of the newly allocated file
    if (unallocated.size > size) {
      unallocated.size -= newfile.stride();
      write_header(unallocated.offset+newfile.stride(), unallocated);
    }
    return write_header(newfile_pos, newfile);
  }
  
  // remove file <file_id> from disk
  bool remove(byte file_id) {
    // file_id == 0 is unallocated space, can't be removed...
    if (file_id == 0)
      return false;
    // find the file to be deleted and its immediate neighbors
    microfsfile prev, cur, next;
    find_contiguous_files(file_id, &prev, &cur, &next);
    if (!cur.is_valid())
      return false;
    // check if we can merge the file with one or more of its neighbors
    size_t new_size, new_pos;
    if (prev.mergeable() && next.mergeable()) {
      new_size = ((size_t)prev.size) + cur.stride() + next.stride();
      new_pos = prev.offset;
    } else if (prev.mergeable()) {
      new_size = ((size_t)prev.size) + cur.stride();
      new_pos = prev.offset;
    } else if (next.mergeable()) {
      new_size = ((size_t)cur.size) + next.stride();
      new_pos = cur.offset;
    } else {
      new_size = ((size_t)cur.size);
      new_pos = cur.offset;
    }
    // actually write to disk the changes
    // FIXME: proceed backwards
    /* CRITICAL SECTION */ {
      while (new_size > 255) {
        new_pos += write_header(new_pos, microfsfile(0, 255)).stride();
        new_size -= 255;
      }
      write_header(new_pos, microfsfile(0, new_size));
    }
    return true;
  }
  
  private:

  // find a random chunk of eeprom, at least size+2 bytes long
  microfsfile find_alloc(byte size) {
    size_t pos = 0, candidates = 0;
    size_t start_pos = rand() % size;
    // try to find an free chunk after a random start_pos (wrapping around, if needed)
    while (pos < size) {
      size_t cur_pos = (start_pos + pos) % size;
      microfsfile f = read_header(cur_pos);
      size_t size_plus_2 = ((size_t)size)+((size_t)2);
      if (f.is_valid() && f.id == 0 && (f.size == size || f.size >= size_plus_2)) {
        return f;
      }
      pos += f.stride();
    }
    return microfsfile();    
  }
  
  // find an unused file id in eeprom
  byte find_id() {
    size_t pos = 0;
    byte mask[256/8] = {0};
    while (pos < size) {
      microfsfile f = read_header(pos);
      mask[f.id/8] |= ((byte)1) << (f.id%8);
      pos += f.stride();
    }
    for (int id=1; id<256; id++) {
      byte used = mask[id/8] & (((byte)1) << (id%8));
      if (!used) {
        return id;
      }
    }
    return 0;
  }
  
  void find_contiguous_files(byte file_id, microfsfile *prev, microfsfile *cur, microfsfile *next) {
    size_t pos = 0;
    microfsfile prev_f = microfsfile();
    *prev = *cur = *next = microfsfile();
    while (pos < size) {
      microfsfile f = read_header(pos);
      if (f.is_valid() && f.id == file_id) {
        *cur = f;
        *prev = prev_f;
        *next = read_header(pos + f.stride());
        return;
      }
      prev_f = f;
      pos += f.stride();
    }    
  }
  
  // read and interpret the two bytes at pos+0 and pos+1 in EEPROM as a file header
  // note: no check is performed about pos pointing to an actual file header!
  microfsfile read_header(size_t pos) {
    if (pos < 0 || pos+2 > size)
      return microfsfile();
    byte file_id = eeprom_read(pos+0);  
    byte file_size = eeprom_read(pos+1);  
    return microfsfile(file_id, file_size, pos);
  }
  
  microfsfile write_header(size_t pos, microfsfile f) {
    if (pos < 0 || pos+2 > size)
      return f;
    /* CRITICAL SECTION */ {
      eeprom_update(pos+1, f.size);
      eeprom_update(pos+0, f.id);
    }
    f.offset = pos;
    return f;
  }
  
};

microfs fs;

#endif // MICROFS
