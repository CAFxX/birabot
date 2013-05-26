template <class I, int N>
class wrapping {
  I value;
  public:
  wrapping() : value(0) {
  }
  wrapping(I v) {
    set(v);
  }
  wrapping<I, N> & operator+=(I n) {
    return set(value + n);
  }
  wrapping<I, N> & operator-=(I n) {
    return set(value - n);
  }
  wrapping<I, N> & operator++() {
    value = value == N-1 ? 0 : value+1;
    return *this;
  }
  wrapping<I, N> & operator--() {
    value = value == 0 ? N-1 : value-1;
    return *this;
  }
  wrapping<I, N> operator++(int) {
    wrapping<I, N> pv = value;
    ++(*this);
    return pv;
  }
  wrapping<I, N> operator--(int) {
    wrapping<I, N> pv = value;
    --(*this);
    return pv;
  }
  wrapping<I, N> operator+(I n) {
    wrapping<I, N> nv = value;
    return nv += n;
  }
  wrapping<I, N> operator-(I n) {
    wrapping<I, N> nv = value;
    return nv -= n;
  }  
  operator I() {
    return value;
  }
  private:
  wrapping<I, N> set(I n) {
    while (n < 0) {
      n += N;
    }
    if (n >= N) {
      n %= N;
    }
    value = n;
    return *this;
  }
};


