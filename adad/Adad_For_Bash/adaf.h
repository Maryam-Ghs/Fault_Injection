#ifndef ADAF_H
#define ADAF_H

#include <stdio.h>
#include <ostream>

class adaf {
protected:
  float	value;
public:
  adaf() {
    value = 0.0f;
  };
  adaf(const adaf& val) {
    value = val.value;
  };
  adaf(float val) {
    value = val;
  };
  ~adaf() {
  };
  float val() const {
    return value;
  };
  void set(float val) {
    value = val;
  };
  operator float() const { return value; }

  // assignment
  adaf& operator= (float dval);
  adaf& operator= (const adaf& val);

  // unary + and -
  adaf operator+ () const;
  adaf operator- () const;

  // binary +
  void operator+= (const adaf &a);
  adaf operator+ (const adaf &a) const;
  void operator+= (float dval);
  adaf operator+ (float dval) const;
  friend adaf operator+ (float dval, const adaf &a);

  // binary -
  void operator-= (const adaf &a);
  adaf operator- (const adaf &a) const;
  void operator-= (float dval);
  adaf operator- (float dval) const;
  friend adaf operator- (float dval, const adaf &a);

  // binary *
  void operator*= (const adaf &a);
  adaf operator* (const adaf &a) const;
  void operator*= (float dval);
  adaf operator* (float dval) const;
  friend adaf operator* (float dval, const adaf &a);

  // binary /
  void operator/= (const adaf &a);
  adaf operator/ (const adaf &a) const;
  void operator/= (float dval);
  adaf operator/ (float dval) const;
  friend adaf operator/ (float dval, const adaf &a);

  // relational operators
  bool operator== (const adaf &a) const;
  bool operator!= (const adaf &a) const;
  bool operator< (const adaf &a) const;
  bool operator> (const adaf &a) const;
  bool operator<= (const adaf &a) const;
  bool operator>= (const adaf &a) const;
  bool operator== (float dval) const;
  bool operator!= (float dval) const;
  bool operator< (float dval) const;
  bool operator> (float dval) const;
  bool operator<= (float dval) const;
  bool operator>= (float dval) const;
  friend bool operator== (float dval, const adaf &a);
  friend bool operator!= (float dval, const adaf &a);
  friend bool operator< (float dval, const adaf &a);
  friend bool operator> (float dval, const adaf &a);
  friend bool operator<= (float dval, const adaf &a);
  friend bool operator>= (float dval, const adaf &a);
  friend std::ostream& operator<<(std::ostream& os, const adaf& a);

  // function calls
  friend adaf expf(const adaf &a);
  // function calls
  friend adaf sqrtf(const adaf &a);
    // function calls
  friend adaf exp(const adaf &a);
  // function calls
  friend adaf sqrt(const adaf &a);
  friend adaf max(const adaf& a, float b);
  friend adaf max(float a, const adaf& b);
  friend adaf min(const adaf& a, float b);
  friend adaf min(float a, const adaf& b);
};

class adaf_array {
private:
  adaf	*DATA;
  int	 size;
public:
  adaf_array(int N);
  adaf_array(float *A, int N);
  adaf &operator[](adaf jndex);
  adaf &operator[](int index);
};

// functions in fault.cpp
extern float get_faulty_float(int id, float value);
extern int get_faulty_boolean(int id, int value);

// functions in main.cpp
extern void coredump(int index, int lb, int ub);
  
#endif /* ADAF_H */
