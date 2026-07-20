#include <stdio.h>
#include <string>
#include <map>
#include "adad.h"

extern int get_location_id(const char *);

// ADAD Operator implementations
adad& adad::operator=(int dval) {
  int id = get_location_id("A=K");
  value = get_faulty_integer(id, dval);
  return *this;
}

adad& adad::operator=(const adad &v) { 
  int id = get_location_id("A=A");
  value = get_faulty_integer(id, v.value);
  return *this;
}

// prefix increment
adad& adad::operator++ () {
  int id = get_location_id("++A");
  value = get_faulty_integer(id, ++value);
  return *this;
}

// prefix decrement
adad& adad::operator-- () {
  int id = get_location_id("--A");
  value = get_faulty_integer(id, --value);
  return *this;
}

// postfix increment
adad adad::operator++(int) {
  adad r = *this;  // save old value
  int id = get_location_id("A++");
  value = get_faulty_integer(id, ++value);  // increment internal int
  return r;        // return old value
}

// postfix decrement 
adad adad::operator--(int) {
  adad r = *this;  // save old value
  int id = get_location_id("A--");
  value = get_faulty_integer(id, --value);  // decrement internal int
  return r;        // return old value
}

// unary + and -
adad adad::operator+ () const { 
  adad   r = *this;
  int id = get_location_id("+A");
  r.value = get_faulty_integer(id, +value);
  return(r);
}

adad adad::operator- () const {
  adad   r = *this;
  int id = get_location_id("-A");
  r.value = get_faulty_integer(id, -value);
  return(r);
}

// binary +
adad& adad::operator+= (const adad &a) {
  int id = get_location_id("A+=A");
  value = get_faulty_integer(id, value + a.value);  
  return *this;
}

adad adad::operator+ (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A+A");
  r.value = get_faulty_integer(id, r.value + a.value);  
  return(r);
}

adad& adad::operator+= (int dval) {
  int id = get_location_id("A+=K");
  value = get_faulty_integer(id, value + dval);  
  return *this;
}

adad adad::operator+ (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A+K");
  r.value = get_faulty_integer(id, r.value+dval);
  return(r);
}

adad operator+ (int dval, const adad &a) {
  adad   r;
  int id = get_location_id("A=K+A");
  r.value = get_faulty_integer(id, dval+a.value);
  return(r);
}

// binary -
adad& adad::operator-= (const adad &a) {
  int id = get_location_id("A-=A");
  value = get_faulty_integer(id, value - a.value);
  return *this;
}

adad adad::operator- (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A-A");
  r.value = get_faulty_integer(id, r.value - a.value);
  return(r);
}

adad& adad::operator-= (int dval) {
  int id = get_location_id("A-=K");
  value = get_faulty_integer(id, value - dval);
  return *this;
}

adad adad::operator- (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A-K");
  r.value = get_faulty_integer(id, r.value - dval);
  return(r);
}

adad operator- (int dval, const adad &a) {
  adad   r;
  int id = get_location_id("A=K-A");
  r.value = get_faulty_integer(id, dval - a.value);
  return(r);
}

// binary *
adad& adad::operator*= (const adad &a) {
  int id = get_location_id("A*=A");
  value = get_faulty_integer(id, value * a.value);
  return *this;
}

adad adad::operator* (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A*A");
  r.value = get_faulty_integer(id, r.value * a.value);
  return(r);
}

adad& adad::operator*= (int dval) {
  int id = get_location_id("A*=K");
  value = get_faulty_integer(id, value * dval);
  return *this;
}

adad adad::operator* (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A*K");
  r.value = get_faulty_integer(id, r.value * dval);
  return(r);
}

adad operator* (int dval, const adad &a) {
  adad   r;
  int id = get_location_id("A=K*A");
  r.value = get_faulty_integer(id, dval * a.value);
  return(r);
}

// binary /
adad& adad::operator/= (const adad &a) {
  int id = get_location_id("A/=A");
  value = get_faulty_integer(id, value / a.value);
  return *this;
}

adad adad::operator/ (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A/A");
  r.value = get_faulty_integer(id, r.value / a.value);
  return(r);
}

adad& adad::operator/= (int dval) {
  int id = get_location_id("A/=K");
  value = get_faulty_integer(id, value / dval);
  return *this;
}

adad adad::operator/ (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A/K");
  r.value = get_faulty_integer(id, r.value / dval);
  return(r);
}

adad operator/ (int dval, const adad &a) {
  adad r;
  int id = get_location_id("A=K/A");
  r.value = get_faulty_integer(id, dval / a.value);
  return(r);
}

// binary ^
adad& adad::operator^= (const adad &a) {
  int id = get_location_id("A^=A");
  value = get_faulty_integer(id, value ^ a.value);
  return *this;
}

adad adad::operator^ (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A^A");
  r.value = get_faulty_integer(id, r.value ^ a.value);
  return(r);
}

adad& adad::operator^= (int dval) {
  int id = get_location_id("A^=K");
  value = get_faulty_integer(id, value ^ dval);
  return *this;
}

adad adad::operator^ (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A^K");
  r.value = get_faulty_integer(id, r.value ^ dval);
  return(r);
}

adad operator^ (int dval, const adad &a) {
  adad   r;
  int id = get_location_id("A=K^A");
  r.value = get_faulty_integer(id, dval ^ a.value);
  return(r);
}

// binary >>
adad& adad::operator>>= (const adad &a) {
  int id = get_location_id("A>>=A");
  value = get_faulty_integer(id, value >> a.value);
  return *this;
}

adad adad::operator>> (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A>>A");
  r.value = get_faulty_integer(id, r.value >> a.value);
  return(r);
}

adad& adad::operator>>= (int dval) {
  int id = get_location_id("A>>=K");
  value = get_faulty_integer(id, value >> dval);
  return *this;
}

adad adad::operator>> (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A>>K");
  r.value = get_faulty_integer(id, r.value >> dval);
  return(r);
}

adad operator>> (int dval, const adad &a) {
  adad   r;
  int id = get_location_id("A=K>>A");
  r.value = get_faulty_integer(id, dval >> a.value);
  return(r);
}

// binary <<
adad& adad::operator<<= (const adad &a) {
  int id = get_location_id("A<<=A");
  value = get_faulty_integer(id, value << a.value);
  return *this;
}

adad adad::operator<< (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A<<A");
  r.value = get_faulty_integer(id, r.value << a.value);
  return(r);
}

adad& adad::operator<<= (int dval) {
  int id = get_location_id("A<<=K");
  value = get_faulty_integer(id, value << dval);
  return *this;
}

adad adad::operator<< (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A<<K");
  r.value = get_faulty_integer(id, r.value << dval);
  return(r);
}

adad operator<< (int dval, const adad &a) {
  adad   r;
  int id = get_location_id("A=K<<A");
  r.value = get_faulty_integer(id, dval << a.value);
  return(r);
}

// binary %
adad& adad::operator%= (const adad &a) {
  int id = get_location_id("A%=A");
  value = get_faulty_integer(id, value % a.value);
  return *this;
}

adad adad::operator% (const adad &a) const {
  adad   r = *this;
  int id = get_location_id("A=A%A");
  r.value = get_faulty_integer(id, r.value % a.value);
  return(r);
}

adad& adad::operator%= (int dval) {
  int id = get_location_id("A%=K");
  value = get_faulty_integer(id, value % dval);
  return *this;
}

adad adad::operator% (int dval) const {
  adad   r = *this;
  int id = get_location_id("A=A%K");
  r.value = get_faulty_integer(id, r.value % dval);
  return(r);
}

adad operator% (int dval, const adad &a) {
  adad   r;
  int id = get_location_id("A=K%A");
  r.value = get_faulty_integer(id, dval % a.value);
  return(r);
}

// relational operators
int adad::operator== (const adad &a) const {
  int id = get_location_id("A==A");
  return get_faulty_boolean(id, value == a.value);
}

int adad::operator!= (const adad &a) const {
  int id = get_location_id("A!=A");
  return get_faulty_boolean(id, value != a.value);
}

int adad::operator< (const adad &a) const {
  int id = get_location_id("A<A");
  return get_faulty_boolean(id, value < a.value);
}

int adad::operator> (const adad &a) const {
  int id = get_location_id("A>A");
  return get_faulty_boolean(id, value > a.value);
}

int adad::operator<= (const adad &a) const {
  int id = get_location_id("A<=A");
  return get_faulty_boolean(id, value <= a.value);
}

int adad::operator>= (const adad &a) const {
  int id = get_location_id("A>=A");
  return get_faulty_boolean(id, value >= a.value);
}

int adad::operator== (int dval) const {
  int id = get_location_id("A==K");
  return get_faulty_boolean(id, value == dval);
}

int adad::operator!= (int dval) const {
  int id = get_location_id("A!=K");
  return get_faulty_boolean(id, value != dval);
}

int adad::operator< (int dval) const {
  int id = get_location_id("A<K");
  return get_faulty_boolean(id, value < dval);
}

int adad::operator> (int dval) const {
  int id = get_location_id("A>K");
  return get_faulty_boolean(id, value > dval);
}

int adad::operator<= (int dval) const {
  int id = get_location_id("A<=K");
  return get_faulty_boolean(id, value <= dval);
}

int adad::operator>= (int dval) const {
  int id = get_location_id("A>=K");
  return get_faulty_boolean(id, value >= dval);
}

int operator== (int dval, const adad &a) {
  int id = get_location_id("K==A");
  return get_faulty_boolean(id, dval == a.value);
}

int operator!= (int dval, const adad &a) {
  int id = get_location_id("K!=A");
  return get_faulty_boolean(id, dval != a.value);
}

int operator< (int dval, const adad &a) {
  int id = get_location_id("K<A");
  return get_faulty_boolean(id, dval < a.value);
}

int operator> (int dval, const adad &a) {
  int id = get_location_id("K>A");
  return get_faulty_boolean(id, dval > a.value);
}

int operator<= (int dval, const adad &a) {
  int id = get_location_id("K<=A");
  return get_faulty_boolean(id, dval <= a.value);
}

int operator>= (int dval, const adad &a) {
  int id = get_location_id("K>=A");
  return get_faulty_boolean(id, dval >= a.value);
}

adad_array::adad_array(int N) {
  DATA = new adad[N];
  size = N;
  for (int i=0; i<N; i++) {
    DATA[i].set( 0 );
  }
}

adad_array::adad_array(int *A, int N) {
  DATA = new adad[N];
  size = N;
  for (int i=0; i<N; i++) {
    DATA[i].set( A[i] );
  }
}

adad &adad_array::operator[](int index) {
  if (index < 0 || index >= size) {
    // if the index is out of bounds, we assume a core dump
    coredump(index, 0, size);
  }
  return DATA[index];
}

adad &adad_array::operator[](adad jndex) {
  if (jndex.val() < 0 || jndex.val() >= size) {
    // if the index is out of bounds, we assume a core dump
    coredump(jndex.val(), 0, size);
  }
  return DATA[jndex.val()];
}
