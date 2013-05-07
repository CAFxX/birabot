#include <stddef.h>
#include <typeinfo>

class ux {
  friend class uxmgr;
  protected:
  ux *back;
  ux() : back(NULL) {}
  public:
  virtual void draw() = 0;
  virtual void on_key(char key) {};
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
    if (curr != NULL && back != NULL)
      delete curr;
    curr = new T();
    curr->back = back;
  }

  template <class T>
  static void show(ux *back = NULL) {
    get()._show<T>(back);
  }
  
  void _back() {
    ux *back = curr->back;
    if (back == NULL)
      return;
    curr->back = NULL;
    delete curr;
    curr = back;
  }
  
  static void back() {
    get()._back();
  }

  void draw() {
    curr->draw();
  }
  
  void on_key(char key) {
    curr->on_key(key);
  }
  
};


