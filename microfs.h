/* 
  MicroFS
  A low-footprint wear-leveling file system for Arduino/AVR microcontrollers
  
  features:
  - 2 byte overhead per file
  - (optional) wear-levelling
  
  limits:
  - 255 files (files are unnamed, can be identified using a number from 1 to 255 inclusive)
  - 256 bytes per file
  - no directories, ACLs, file[cma]time, etc.
  - files are allocated as contiguos chunks of EEPROM, file fragments are not allowed 
    (this implies that to allocate a file of size N bytes, a chunk of size N+2 must be free)
  
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
  - use EEPROMex or other (better) libraries instead of the standard EEPROM library
  - files can't be changed in size (extend in place, copy-and-extend)
  - free space defragmentation
  - check after write to ensure successful writes
*/

#ifndef MICROFS
#define MICROFS 

class microfsfile {
  
  friend class microfs;
  
  // IN-EEPROM MEMBERS //
  byte id; // id of the file, id 0 is reserved for unallocated space
  byte size; // size of file (bytes)
  
  // IN-MEMORY MEMBERS //
  size_t offset; // offset of file header in EEPROM

  public:
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
  
  bool is_valid() {
    return offset >= 0; // FIXME  
  }
  
  byte get_size() {
    return size;
  }
  
  bool write_byte(byte pos, byte value) {
    if (!is_valid())
      return false;
    size_t off = offset + 2 + pos;
    if (off < 0 || off >= size)
      return false;
    EEPROM.write(off, value);
    return true;
  }
  
  byte read_byte(byte pos) {
    if (!is_valid())
      return 0;
    size_t off = offset + 2 + pos;
    if (off < 0 || off >= size)
      return 0;
    return EEPROM.read(off);
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
  
  size_t stride() {
    return 2 + size;
  }
  
};

class microfs {
  
  size_t size;
  bool consistent;
  
  public:
  microfs() {
    size = E2END + 1;
    consistent = check_disk();
  }
  
  void format() {
    microfsfile unallocated(0, 256);
    size_t pos = 0;
    while (pos < size) {
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
    while (pos+2 < size) {
      microfsfile f = read_header(pos);
      if (f.is_valid() && f.id == file_id) {
        return f;
      }
      pos += f.stride();
    }
    return microfsfile();
  }
  
  microfsfile create(byte size) {
    microfsfile unallocated = find_alloc(size);
    if (!unallocated.is_valid())
      return microfsfile();
    byte id = find_id();
    if (id == 0)
      return microfsfile();
    microfsfile newfile(id, size);
    size_t newfile_pos = unallocated.offset;
    if (size < unallocated.size) {
      unallocated.size -= newfile.stride();
      write_header(unallocated.offset+newfile.stride(), unallocated);
    }
    return write_header(newfile_pos, newfile);
  }
  
  bool remove(byte file_id) {
    if (file_id == 0)
      return false;
    microfsfile prev, cur, next;
    find_contiguous_files(file_id, &prev, &cur, &next);
    if (!cur.is_valid())
      return false;
    if (prev.is_valid() && prev.id == 0 && next.is_valid() && next.id == 0) {
      prev.size = prev.size + cur.stride() + next.stride();
      write_header(prev.offset, prev);
    } else if (prev.is_valid() && prev.id == 0) {
      prev.size = prev.size + cur.stride();
      write_header(prev.offset, prev);
    } else if (next.is_valid() && next.id == 0) {
      cur.size = cur.size + next.stride();
      cur.id = 0;
      write_header(cur.offset, cur);
    } else {
      cur.id = 0;
      write_header(cur.offset, cur);
    }
    return true;
  }
  
  private:

  // find a random chunk of eeprom, at least size+2 bytes long
  microfsfile find_alloc(byte size) {
    size_t pos = 0, candidates = 0;
    size_t start_pos = rand() % size;
    // try to find an free chunk after a random start_pos (with wrap around)
    while (pos < size) {
      size_t cur_pos = (start_pos + pos) % size;
      microfsfile f = read_header(cur_pos);
      if (f.is_valid() && f.id == 0 && (f.size == size || f.size >= size+2)) {
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
  
  void write_raw(size_t pos, void *src, size_t len) {
    byte *_src = (byte*)src;
    for (size_t i=0; i<len; i++)
      EEPROM.write(pos+i, _src[i]);
  }
  
  microfsfile read_header(size_t pos) {
    return pos+2 < size ? microfsfile(EEPROM.read(pos+0), EEPROM.read(pos+1), pos) : microfsfile();
  }
  
  microfsfile write_header(size_t pos, microfsfile f) {
    f.offset = pos;
    EEPROM.write(pos+1, f.size);
    EEPROM.write(pos+0, f.id);
    return f;
  }
  
};

#endif // MICROFS
