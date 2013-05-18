
class Step {
  public:
  byte duration;
  boolean constant:1;
  byte temperature:7;
  Step() : duration(0), constant(true), temperature(0) {}
};

class Program {
  
  byte file_id;
  byte *ptr;
  byte size;
  
  public:
  Program(byte file_id=0) {
    this->file_id = file_id;
    ptr = NULL;
    size = 0;
    microfsfile f = fs.open(file_id);
    if (f.is_valid()) {
      alloc(f.get_size());
      f.read_bytes(0, ptr, f.get_size());
    }
  }
  
  ~Program() {
    if (ptr != NULL) {
      free(ptr);
      ptr = NULL;
    }
  }
  
  bool alloc(byte size) {
    byte *newptr = (byte*)realloc(ptr, size);
    this->size = size;
    ptr = newptr;
  }
  
  bool saveChanges() {
    fs.remove(file_id);
    microfsfile f = fs.create(size, file_id);
    if (!f.is_valid()) {
      return false;
    }
    f.write_bytes(0, ptr, size);
    return true;
  }
  
  byte steps() {
    return size/2; // FIXME
  }
  
  int duration() {
    int minutes = 0;
    for (int i=0; i<steps(); i++) {
      minutes += getDuration(i);
    }
    return minutes;
  }
  
  bool addStep(byte pos) {
    if (size >= 254)
      return false;
    if (pos > steps())
      return false;
    alloc(size+2);
    for (int i=steps()-1; i>pos; i--) {
      getStep(i) = getStep(i-1);
    }
    getStep(pos) = Step();
  }
  
  bool delStep(byte pos) {
    if (size < 2)
      return false;
    if (pos >= steps())
      return false;
    for (int i=pos+1; i<steps(); i++) {
      getStep(i-1) = getStep(i);
    }
    alloc(size-2);
  }
  
  bool addStep() {
    return addStep(steps());
  }
  
  bool delStep() {
    return delStep(steps()-1);
  }
  
  Step& getStep(byte pos) {
    return *(Step*)(void*)(ptr+pos*2);
  }
  
  byte getDuration(byte pos) {
    return getStep(pos).duration;
  }
  
  void setDuration(byte pos, byte duration) {
    getStep(pos).duration = duration;
  }
  
  byte getTemperature(byte pos) {
    return getStep(pos).temperature;
  }
  
  void setTemperature(byte pos, byte temperature) {
    getStep(pos).temperature = temperature;
  }
  
  byte getMethod(byte pos) {
    return getStep(pos).constant;
  }
  
  void setMethod(byte pos, byte method) {
    getStep(pos).constant = !!method;
  }
  
  byte getTemperatureAt(int minute) {
    if (minute >= duration() || minute < 0) {
      return 0;
    }
    int minutes = 0;
    byte prevTemp = 0;
    for (int i=0; i<steps() && minutes<minute; i++) {
      minutes += getDuration(i);
      prevTemp = getTemperature(i);
    }
    return minutes;

  }
  
};


