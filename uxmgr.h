#ifndef UXMGR
#define UXMGR

#include <stddef.h>
#include <HardwareSerial.h>

#define __string_PGM(data)                  \
  char __buf__[sizeof(data)];               \
  strcpy_P(__buf__, PSTR(data));             

extern HardwareSerial Serial;

class ux {
  friend class uxmgr;
  protected:
  ux *prev;
  ux() : prev(NULL) {}
  virtual ~ux() { delete prev; prev = NULL; }
  template <class T> void show();
  template <class T> void show(int param);
  template <class T> void next();
  template <class T> void next(int param);
  void back();
  void back(int retVal);
  public:
  virtual void on_init(int param) {};
  virtual void on_show() {};
  virtual void draw() = 0;
  virtual void on_key(char key) { back(); }
  virtual void on_back(int retVal) {};
};

template <unsigned max, unsigned delta=max, unsigned min=0>
class ux_input_numeric {
  unsigned value;
  public:
  ux_input_numeric(unsigned initial_value=0) {
    value = initial_value;
  }
  void on_key(char key) {
    if (key < '0' || key > '9') {
      return;
    }
    value = value * 10 + (key - '0');
    if (value >= max) {
      while (value >= delta) {
        value -= delta;
      }
    }
    if (value < min) {
      value = min;
    }
  }
  operator unsigned() {
    return value;
  }
  unsigned operator()() {
    return value;
  }
};


static int freeRam() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

class uxmgr {
  
  static uxmgr singleton;

  ux *curr;

  uxmgr() {
    curr = NULL;
  }
  
  void dump(const char* prefix, bool in) {
    Serial.print(in ? '>' : '<');
    if (prefix != NULL) {
      Serial.print(prefix);
    }
    Serial.print(freeRam());
    Serial.print(',');
    Serial.print((unsigned)curr);
    if (curr != NULL) {
      Serial.print(',');
      Serial.print((unsigned)curr->prev);
    }
    Serial.println();
  }
  
  public:
  
  static uxmgr& get() {
    return singleton;
  }
  
  template <class T>
  void show(ux *prev = NULL) {
    __string_PGM("show");
    dump(__buf__, true);
    if (prev == NULL)
      delete curr;
    curr = new T();
    curr->prev = prev;
    dump(__buf__, false);
    curr->on_show();
  }
  
  template <class T>
  void show(ux *prev, int param) {
    show<T>(prev);
    curr->on_init(param);
  }
  
  template <class T>
  void show(int param) {
    show<T>(NULL, param);
  }
  
  template <class T>
  void next() {
    show<T>(curr);
  }

  template <class T>
  void next(int param) {
    show<T>(curr, param);
  }

  void back(int retVal=0, bool withRetVal=false) {
    __string_PGM("back");
    dump(__buf__, true);
    ux *prev = curr->prev;
    if (prev != NULL) {
      curr->prev = NULL;
      delete curr;
      curr = prev;
      if (withRetVal) {
        curr->on_back(retVal);
      }
      curr->on_show();
    }
    dump(__buf__, false);
  }
  
  void draw() {
    curr->draw();
  }
  
  void on_key(char key) {
    curr->on_key(key);
  }
  
};

inline void ux::back() { 
  uxmgr::get().back(); 
}

inline void ux::back(int retVal) { 
  uxmgr::get().back(retVal, true);
}

template <class T> void ux::show() {
  uxmgr::get().show<T>();
}

template <class T> void ux::show(int param) {
  uxmgr::get().show<T>(param);
}

template <class T> void ux::next() {
  uxmgr::get().show<T>(this);
}

template <class T> void ux::next(int param) {
  uxmgr::get().show<T>(this, param);
}

#endif // UXMGR
