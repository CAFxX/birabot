class microfsfile {
  
  friend class microfs;
  
  byte id;
  byte size;
  size_t offset;

  public: 
  microfsfile(byte file_id, byte file_size) {
    id = file_id;
    size = file_size;
    offset = -1;
  }
  
  microfsfile(byte file_id, byte file_size, size_t eeprom_offset) {
    id = file_id;
    size = file_size;
    offset = eeprom_offset;
  }
  
  bool is_valid() {
    return offset >= 0; // FIXME  
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
  
};

class microfs {
  
  size_t size;
  
  public:
  microfs() {
    size = 1024;
  }
  
  void format() {
    microfsfile unallocated(0, 256);
    size_t pos = 0;
    while (pos < size) {
      write_header(pos, unallocated);
      pos += 2 + 256;
    }
  }
  
  size_t used() {
    size_t pos = 0, used = 0;
    while (pos+2 < size) {
      microfsfile f = read_header(pos);
      if (f.id != 0) {
        used += f.size;
      }
      pos += f.size + 2;
    }
    return used;
  }
  
  microfsfile open(byte file_id) {
    size_t pos = 0;
    while (pos+2 < size) {
      microfsfile f = read_header(pos);
      if (f.id == file_id) {
        return f;
      }
      pos += f.size + 2;
    }
    return microfsfile(0, 0);
  }
  
  microfsfile create(byte size) {
    size_t pos = find_alloc(size);
    if (pos == -1)
      return microfsfile(0, 0);
    byte id = find_id();
    if (id == 0)
      return microfsfile(0, 0);
    microfsfile newfile(id, size);
    return write_header(pos, newfile);
  }
  
  bool delete(byte file_id) {
    microfsfile f = open(file_id)
    if (!f.is_valid())
      return false;
    
  }
  
  private:

  // find a random chunk of eeprom, at least size+2 bytes long
  size_t find_alloc(byte size) {
    size_t pos = 0, candidates = 0;
    size_t start_pos = rand() % size;
    while (pos+2 < size) {
      microfsfile f = read_header(pos);
      if (pos >= start_pos && f.id == 0 && f.size >= size) {
        return pos;
      }
      pos += f.size + 2;
    }
    while (pos+2 < size) {
      microfsfile f = read_header(pos);
      if (f.id == 0 && f.size >= size) {
        return pos;
      }
      pos += f.size + 2;
    }    
    return -1;    
  }
  
  // find an unused file id in eeprom
  byte find_id() {
    size_t pos = 0;
    byte mask[256/8] = {0};
    while (pos+2 < size) {
      microfsfile f = read_header(pos);
      mask[f.id/8] |= ((byte)1) << (f.id%8);
      pos += f.size + 2;
    }
    for (int id=1; id<256; id++) {
      byte used = mask[id/8] & (((byte)1) << (id%8));
      if (!used) {
        return id;
      }
    }
    return 0;
  }  
  
  void write_raw(size_t pos, void *src, size_t len) {
    byte *_src = (byte*)src;
    for (size_t i=0; i<len; i++)
      EEPROM.write(pos+i, _src[i]);
  }
  
  microfsfile read_header(size_t pos) {
    return microfsfile(EEPROM.read(pos+0), EEPROM.read(pos+1), pos);
  }
  
  microfsfile write_header(size_t pos, microfsfile f) {
    f.offset = pos;
    EEPROM.write(pos+1, f.size);
    EEPROM.write(pos+0, f.id);
    return f;
  }
  
};


