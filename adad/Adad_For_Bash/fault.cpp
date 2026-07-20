#include <stdlib.h>
#include <string>
#include <map>
#include <execinfo.h>
#include "adad.h"

extern std::map<std::string, int> fault_strings;
extern std::map<int, double>      fault_probs;
extern std::map<int, int>         fault_counts, run_counts;
extern int                        num_sdc, num_core, last_id, fault_cycle, fault_type;

extern int                   backtrace(void **buffer, int size);
extern char**                backtrace_symbols(void* const* array, int size);


int get_location_id(const char *OP) {
    void *p[3];
    char **q;
    backtrace(p, 3);
    q = backtrace_symbols(p, 3);

    std::string op = OP;
    std::string loc = q[2];   // function + file + offset

    std::string S = op + "\t" + loc;

    free(q);

    int id;
    auto it = fault_strings.find(S);
    if (it == fault_strings.end()) {
        id = 1 + fault_strings.size();
        fault_strings[S] = id;
    } else {
        id = it->second;
    }

    return id;
}

static double rand01() {
  return random() * 4.656612e-10;
}

int get_faulty_integer(int id, int value) {
  int f, g;
  ++run_counts[id];
  // has the location been marked for fault insertion
  if (fault_probs.find(id) == fault_probs.end()) {
    return value;
  }
  // is the probability sensible (may not be needed)
  if (fault_probs[id] <= 0.0 || fault_probs[id] > 1.0) {
    return value;
  }
  // roll the dice
  if (rand01() > fault_probs[id]) {
    return value;
  }
  if ((fault_cycle != run_counts[id])) {
    return value;
  }
  // increment the fault count
  ++fault_counts[id];
  // mark this as the last location
  last_id = id;
  // pick a random bit location
  f = random() % 32;
  // put a 1 there
  g = 1 << f;
  // xor it on the result
  value = value ^ g;
  return value;
}

int get_faulty_boolean(int id, int value) {
  ++run_counts[id];
  // has the location been marked for fault insertion
  if (fault_probs.find(id) == fault_probs.end()) {
    return value;
  }
  // is the probability sensible (may not be needed)
  if (fault_probs[id] <= 0.0 || fault_probs[id] > 1.0) {
    return value;
  }
  // roll the dice
  if (rand01() > fault_probs[id]) {
    return value;
  }
  if ((fault_cycle != run_counts[id])) {
    return value;
  }
  // increment the fault count
  ++fault_counts[id];
  // mark this as the last location
  last_id = id;
  // and reverse the result
  return ! value;
}

// float get_faulty_float(int id, float value) {
//   int f, g;
//   union two {
//     int   i;
//     float f;
//   } x;
//   ++run_counts[id];
//   // has the location been marked for fault insertion
//   if (fault_probs.find(id) == fault_probs.end()) {
//     return value;
//   }
//   // is the probability sensible (may not be needed)
//   if (fault_probs[id] <= 0.0 || fault_probs[id] > 1.0) {
//     return value;
//   }
//   // roll the dice
//   if (rand01() > fault_probs[id]) {
//     return value;
//   }
//   if ((fault_cycle != run_counts[id])) {
//     return value;
//   }
//   // increment the fault count
//   ++fault_counts[id];
//   // mark this as the last location
//   last_id = id;
//   // pick a random bit location
//   f = random() % 32;
//   // put a 1 there
//   g = 1 << f;
//   // xor it on the result
//   x.f = value;
//   x.i = x.i ^ g;
//   return x.f;
// }

float get_faulty_float(int id, float value) {
  int f, g;

  union {
    int   i;
    float f;
  } x;

  ++run_counts[id];

  if (fault_probs.find(id) == fault_probs.end()) return value;
  if (fault_probs[id] <= 0.0 || fault_probs[id] > 1.0) return value;
  if (rand01() > fault_probs[id]) return value;
  if ((fault_cycle != run_counts[id])) return value;

  ++fault_counts[id];
  last_id = id;

  // controlled bit selection
  switch (fault_type) {
    //0: MANTISSA, 1: EXPONENT, 2: SIGN, 3: ANY
    case 0:
      f = random() % 23;
      break;
    case 1:
      f = 23 + (random() % 8);
      break;
    case 2:
      f = 31;
      break;
    default:
      f = random() % 32;
  }

  g = 1 << f;

  x.f = value;
  x.i ^= g;

  return x.f;
}