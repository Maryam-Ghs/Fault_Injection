#include <execinfo.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <map>
#include <string>

#include "adad.h"

extern std::map<std::string, int> fault_strings;
extern std::map<int, double> fault_probs;
extern std::map<int, int> fault_counts;
extern std::map<int, int> run_counts;
extern std::map<int, std::string> location_kinds;
extern std::map<int, unsigned long long> first_events;
extern int last_id;
extern int fault_cycle;
extern std::string fault_mode;
extern bool fault_injected;
extern unsigned long long global_event_count;
extern std::chrono::steady_clock::time_point pain_start;

int get_location_id(const char *operation) {
  void *frames[3];
  const int frame_count = backtrace(frames, 3);
  char **symbols = backtrace_symbols(frames, frame_count);
  const std::string location =
      frame_count >= 3 && symbols != NULL ? symbols[2] : "N/A";
  free(symbols);

  const std::string key = std::string(operation) + "\t" + location;
  const auto existing = fault_strings.find(key);
  if (existing != fault_strings.end()) {
    return existing->second;
  }
  const int id = static_cast<int>(fault_strings.size()) + 1;
  fault_strings[key] = id;
  return id;
}

static int record_event(int id, const char *kind) {
  ++global_event_count;
  const int occurrence = ++run_counts[id];
  if (first_events.find(id) == first_events.end()) {
    first_events[id] = global_event_count;
  }
  location_kinds[id] = kind;
  return occurrence;
}

static bool should_inject(int id, int occurrence, const char *required_mode) {
  if (fault_injected || fault_mode != required_mode) {
    return false;
  }
  const auto selected = fault_probs.find(id);
  if (selected == fault_probs.end() || selected->second <= 0.0) {
    return false;
  }
  return occurrence == fault_cycle;
}

static void report_injection(int id, int occurrence, int bit_index,
                             const char *kind, const char *before,
                             const char *after) {
  fault_injected = true;
  ++fault_counts[id];
  last_id = id;
  const auto now = std::chrono::steady_clock::now();
  const unsigned long long elapsed_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(now - pain_start)
          .count();
  fprintf(stderr,
          "FI_INJECT\tid=%d\tkind=%s\tmode=%s\toccurrence=%d\t"
          "global_event=%llu\telapsed_ns=%llu\tbit=%d\tbefore=%s\tafter=%s\n",
          id, kind, fault_mode.c_str(), occurrence, global_event_count,
          elapsed_ns, bit_index, before, after);
  fflush(stderr);
}

int get_faulty_integer(int id, int value) {
  static_assert(sizeof(int) == sizeof(uint32_t), "ADAD requires 32-bit int");
  const int occurrence = record_event(id, "integer");

  int bit_index = -1;
  if (fault_mode == "int_lsb") bit_index = 0;
  if (fault_mode == "int_middle") bit_index = 15;
  if (fault_mode == "int_msb") bit_index = 31;
  if (bit_index < 0 || !should_inject(id, occurrence, fault_mode.c_str())) {
    return value;
  }

  uint32_t before_bits;
  memcpy(&before_bits, &value, sizeof(before_bits));
  const uint32_t after_bits = before_bits ^ (UINT32_C(1) << bit_index);
  int result;
  memcpy(&result, &after_bits, sizeof(result));

  char before[16];
  char after[16];
  snprintf(before, sizeof(before), "0x%08x", before_bits);
  snprintf(after, sizeof(after), "0x%08x", after_bits);
  report_injection(id, occurrence, bit_index, "integer", before, after);
  return result;
}

int get_faulty_boolean(int id, int value) {
  const int occurrence = record_event(id, "boolean");
  if (!should_inject(id, occurrence, "boolean")) {
    return value;
  }
  const int result = !value;
  char before[4];
  char after[4];
  snprintf(before, sizeof(before), "%d", value ? 1 : 0);
  snprintf(after, sizeof(after), "%d", result ? 1 : 0);
  report_injection(id, occurrence, -1, "boolean", before, after);
  return result;
}

float get_faulty_float(int id, float value) {
  static_assert(sizeof(float) == sizeof(uint32_t), "ADAF requires 32-bit float");
  const int occurrence = record_event(id, "float");

  int bit_index = -1;
  if (fault_mode == "float_mantissa_middle") bit_index = 11;
  if (fault_mode == "float_exponent_middle") bit_index = 26;
  if (bit_index < 0 || !should_inject(id, occurrence, fault_mode.c_str())) {
    return value;
  }

  uint32_t before_bits;
  memcpy(&before_bits, &value, sizeof(before_bits));
  const uint32_t after_bits = before_bits ^ (UINT32_C(1) << bit_index);
  float result;
  memcpy(&result, &after_bits, sizeof(result));

  char before[16];
  char after[16];
  snprintf(before, sizeof(before), "0x%08x", before_bits);
  snprintf(after, sizeof(after), "0x%08x", after_bits);
  report_injection(id, occurrence, bit_index, "float", before, after);
  return result;
}
