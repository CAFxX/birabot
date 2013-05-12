#include <stddef.h>

static void __uxmgr_back();
static void __uxmgr_back(int retVal);

class ux {
  friend class uxmgr;
  protected:
  ux *prev;
  ux() : prev(NULL) {}
  template <class T> void show();
  template <class T> void show(int param);
  template <class T> void next();
  template <class T> void next(int param);
  void back() { __uxmgr_back(); }
  void back(int retVal) { __uxmgr_back(retVal); }
  public:
  virtual void draw() = 0;
  virtual void on_init(int param) {};
  virtual void on_key(char key) { __uxmgr_back(); }
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
  void _show(int param) {
    _show<T>(NULL, param);
  }

  template <class T>
  void _show(ux *prev = NULL) {
    if (curr != NULL && prev == NULL)
      delete curr;
    curr = new T();
    curr->prev = prev;
  }
  
  template <class T>
  void _show(ux *prev, int param) {
    _show<T>(prev);
    curr->on_init(param);
  }
  
  void _back(int retVal) {
    _back();
    curr->on_back(retVal);
  }

  void _back() {
    ux *prev = curr->prev;
    if (prev == NULL)
      return;
    curr->prev = NULL;
    delete curr;
    curr = prev;
  }
  
  void _draw() {
    curr->draw();
  }
  
  void _on_key(char key) {
    curr->on_key(key);
  }
  
  template <class T>
  static void show(ux *prev = NULL) {
    get()._show<T>(prev);
  }

  template <class T>
  static void show(int param) {
    get()._show<T>(param);
  }

  template <class T>
  static void show(ux *prev, int param) {
    get()._show<T>(prev, param);
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

static void __uxmgr_back() {
  uxmgr::back();
}

static void __uxmgr_back(int retVal) {
  uxmgr::back(retVal);
}

template <class T> void ux::show() {
  uxmgr::show<T>();
}

template <class T> void ux::show(int param) {
  uxmgr::show<T>(param);
}

template <class T> void ux::next() {
  uxmgr::show<T>(this);
}

template <class T> void ux::next(int param) {
  uxmgr::show<T>(this, param);
}




