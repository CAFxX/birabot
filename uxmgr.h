#include <stddef.h>

class ux {
  friend class uxmgr;
  protected:
  ux *back;
  ux() : back(NULL) {}
  public:
  virtual void draw() = 0;
  virtual void on_key(char key) = 0;
  virtual void on_back(int retVal) {};
};

class uxmgr {
  
  static uxmgr singleton;

  ux *curr;

  uxmgr() {
    curr = NULL;
  }
  
  public:
  
  static const char left_arrow = 0x7E;
  
  static uxmgr& get() {
    return singleton;
  }
  
  template <class T>
  void _show(ux *back = NULL) {
    if (curr != NULL && back == NULL)
      delete curr;
    curr = new T();
    curr->back = back;
  }
  
  void _back(int retVal) {
    _back();
    curr->on_back(retVal);
  }

  void _back() {
    ux *back = curr->back;
    if (back == NULL)
      return;
    curr->back = NULL;
    delete curr;
    curr = back;
  }
  
  void _draw() {
    curr->draw();
  }
  
  void _on_key(char key) {
    curr->on_key(key);
  }
  
  template <class T>
  static void show(ux *back = NULL) {
    get()._show<T>(back);
  }

  static void back() {
    get()._back();
  }

  static void back(int retVal) {
    get()._back(retVal);
  }

  static void on_key(char key) {
    get()._on_key(key);
  }
  
  static void draw() {
    get()._draw();
  }
  
};


