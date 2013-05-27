#include "panic.h"

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
  Program(byte file_id=0) : file_id(file_id), ptr(NULL), size(0) {
    Serial.print(F("Program "));
    Serial.println(file_id);
    microfsfile f = fs.open(file_id);
    if (f.is_valid() && file_id != 0) {
      Serial.println(F("loading program"));
      Serial.println(f.get_size());
      alloc(f.get_size());
      byte read = f.read_bytes(0, ptr, f.get_size());
      if (read != f.get_size()) {
        Serial.println(F("error reading program"));
        Serial.println(read);
      }
      for (int i=0; i<steps(); i++) {
        Serial.println(i);
        Serial.println(getMethod(i));
        Serial.println(getDuration(i));
        Serial.println(getTemperature(i));
      }
    }
  }
  
  Program(const Program& other) : file_id(other.file_id) {
    alloc(other.size);
    memcpy(ptr, other.ptr, size);
  }
  
  ~Program() {
    Serial.print(F("~Program "));
    Serial.println(file_id);
    if (ptr != NULL) {
      free(ptr);
      ptr = NULL;
    }
  }
  
  bool is_valid() {
    microfsfile f = fs.open(file_id);
    return f.is_valid() && file_id != 0;
  }
  
  byte id() {
    return file_id;
  }
  
  bool alloc(byte size) {
    byte *newptr = (byte*)realloc(ptr, size);
    if (newptr != NULL) {
      this->size = size;
      ptr = newptr;
      return true;
    }
    Serial.println(F("alloc() fail"));
    return false;
  }
  
  bool saveChanges() {
    Serial.println(F("saveChanges"));
    Serial.println(file_id);
    Serial.println(size);
    fs.remove(file_id);
    Serial.println(F("saveChanges-create"));
    microfsfile f = fs.create(size, file_id);
    if (!f.is_valid()) {
      Serial.println(F("could not create file"));
      return false;
    }
    Serial.println(f.get_id());
    Serial.println(f.get_size());
    Serial.println(f.get_offset());
    Serial.println(F("saveChanges-write"));
    byte len = f.write_bytes(0, ptr, size);
    if (len != size) {
      Serial.println(F("error writing file"));
      Serial.println(len);
      Serial.println(size);
    }
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
    if (pos >= steps()) {
      Serial.println("pos >= steps()");
      Serial.println(pos);
      Serial.println(steps());
    }
    return ((Step*)ptr)[pos];
  }
  
  byte getDuration(byte pos) {
    return getStep(pos).duration;
  }
  
  void setDuration(byte pos, byte duration) {
    Serial.println(F("setDuration"));
    getStep(pos).duration = duration;
  }
  
  byte getTemperature(byte pos) {
    return getStep(pos).temperature;
  }
  
  void setTemperature(byte pos, byte temperature) {
    Serial.println(F("setTemperature"));
    getStep(pos).temperature = temperature;
  }
  
  byte getMethod(byte pos) {
    return getStep(pos).constant;
  }
  
  void setMethod(byte pos, byte method) {
    Serial.println(F("setMethod"));
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
      byte t1 = stepIndex == 0 ? 20 : getTemperature(stepIndex-1);
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


