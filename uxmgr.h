#ifndef UXMGR
#define UXMGR

#include <stddef.h>

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
    while (value >= max) {
      value -= delta;
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

class uxmgr {
  
  static uxmgr singleton;

  ux *curr;

  uxmgr() {
    curr = NULL;
  }
  
  public:
  
  static uxmgr& get() {
    return singleton;
  }
  
  template <class T>
  void show(ux *prev = NULL) {
    if (prev == NULL)
      delete curr;
    curr = new T();
    curr->prev = prev;
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

  void back() {
    ux *prev = curr->prev;
    if (prev == NULL)
      return;
    curr->prev = NULL;
    delete curr;
    curr = prev;
    curr->on_show();
  }
  
  void back(int retVal) {
    back();
    curr->on_back(retVal);
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
  uxmgr::get().back(retVal);
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
