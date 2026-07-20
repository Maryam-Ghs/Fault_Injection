#ifndef ADAD_H
#define ADAD_H

#include <stdio.h>

class adad {
protected:
  int	value;
public:
  adad() {
    value = 0;
  };
  adad(const adad& val) {
    value = val.value;
  };
  adad(int val) {
    value = val;
  };
  ~adad() {
  };
  int val() {
    return value;
  };
  void set(int val) {
    value = val;
  };

  // assignment from int
  // adad& operator=(int v) { value = v; return *this; }

  // Explicit conversion to int
  // explicit operator int() const {
  //   return value;
  // }

  // implicit conversion to int
  operator int() const { return value; }

  // assignment
  adad& operator= (int dval);
  adad& operator= (const adad& val);

  // postfix increment
  adad& operator++ ();
  adad& operator-- ();

  //prefix increment
  adad operator++(int); 
  adad operator--(int); 

  // unary + and -
  adad operator+ () const;
  adad operator- () const;

  // binary +
  adad& operator+= (const adad &a);
  adad operator+ (const adad &a) const;
  adad& operator+= (int dval);
  adad operator+ (int dval) const;
  friend adad operator+ (int dval, const adad &a);

  // binary -
  adad& operator-= (const adad &a);
  adad operator- (const adad &a) const;
  adad& operator-= (int dval);
  adad operator- (int dval) const;
  friend adad operator- (int dval, const adad &a);

  // binary *
  adad& operator*= (const adad &a);
  adad operator* (const adad &a) const;
  adad& operator*= (int dval);
  adad operator* (int dval) const;
  friend adad operator* (int dval, const adad &a);

  // binary /
  adad& operator/= (const adad &a);
  adad operator/ (const adad &a) const;
  adad& operator/= (int dval);
  adad operator/ (int dval) const;
  friend adad operator/ (int dval, const adad &a);

  // binary ^
  adad& operator^= (const adad &a);
  adad operator^ (const adad &a) const;
  adad& operator^= (int dval);
  adad operator^ (int dval) const;
  friend adad operator^ (int dval, const adad &a);

  // binary %
  adad& operator%= (const adad &a);
  adad operator% (const adad &a) const;
  adad& operator%= (int dval);
  adad operator% (int dval) const;
  friend adad operator% (int dval, const adad &a);

  // binary >>
  adad& operator>>= (const adad &a);
  adad operator>> (const adad &a) const;
  adad& operator>>= (int dval);
  adad operator>> (int dval) const;
  friend adad operator>> (int dval, const adad &a);

  // binary <<
  adad& operator<<= (const adad &a);
  adad operator<< (const adad &a) const;
  adad& operator<<= (int dval);
  adad operator<< (int dval) const;
  friend adad operator<< (int dval, const adad &a);

  // relational operators
  int operator== (const adad &a) const;
  int operator!= (const adad &a) const;
  int operator< (const adad &a) const;
  int operator> (const adad &a) const;
  int operator<= (const adad &a) const;
  int operator>= (const adad &a) const;
  int operator== (int dval) const;
  int operator!= (int dval) const;
  int operator< (int dval) const;
  int operator> (int dval) const;
  int operator<= (int dval) const;
  int operator>= (int dval) const;
  friend int operator== (int dval, const adad &a);
  friend int operator!= (int dval, const adad &a);
  friend int operator< (int dval, const adad &a);
  friend int operator> (int dval, const adad &a);
  friend int operator<= (int dval, const adad &a);
  friend int operator>= (int dval, const adad &a);
};

class adad_array {
private:
  adad	*DATA;
  int	 size;
public:
  adad_array(int N);
  adad_array(int *A, int N);
  adad &operator[](adad jndex);
  adad &operator[](int index);
};

// functions in fault.cpp
extern int get_faulty_integer(int id, int value);
extern int get_faulty_boolean(int id, int value);
extern float get_faulty_float(int id, float value);

// functions in main.cpp
extern void coredump(int index, int lb, int ub);
  
#endif /* ADAD_H */
