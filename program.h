
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
    if (newptr != NULL) {
      this->size = size;
      ptr = newptr;
      return true;
    }
    return false;
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
    if (alloc(size+2) == false) {
      return false;
    }
    for (int i=steps()-1; i>pos; i--) {
      getStep(i) = getStep(i-1);
    }
    getStep(pos) = Step();
    return true;
  }
  
  bool delStep(byte pos) {
    if (size < 2)
      return false;
    if (pos >= steps())
      return false;
    for (int i=pos+1; i<steps(); i++) {
      getStep(i-1) = getStep(i);
    }
    return alloc(size-2);
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
  
  byte getStepAt(int minute) {
    for (int i=0, minutes=0; i<steps(); i++) {
      int stepDuration = getDuration(i);
      if (minute < minutes + stepDuration) {
        return i;
      }
      minutes += stepDuration;
    }
    return 0; // FIXME
  }
  
  byte getTemperatureAt(int minute) {
    if (minute >= duration() || minute < 0) {
      return 0;
    }
    byte stepIndex = getStepAt(minute);
    if (getMethod(stepIndex) == 0) {
      byte t1 = stepIndex == 0 ? 0 : getTemperature(stepIndex-1);
      int stepMinute = minute;
      for (int i = 0; i < stepIndex; i++) {
        stepMinute -= getDuration(i);
      }
      return interpolate(t1, getTemperature(stepIndex), stepMinute, getDuration(stepIndex));
    } else {
      return getTemperature(stepIndex);
    }
  }
  
  static byte interpolate(byte t1, byte t2, int minute, int duration) {
    float dt = ((float)t2) - ((float)t1);
    float progress = ((float)minute) / ((float)duration);
    float t = ((float)t1) + dt * progress;
    return t;
  }
  
};


