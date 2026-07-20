#include <getopt.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <chrono>
#include <map>
#include <string>

#include "adad.h"

int array_size = 100;
int num_trials = 1;
int current_trial = 0;
int num_sdc = 0;
int num_core = 0;
int last_id = -1;
int verbose = 0;

char *command = NULL;
jmp_buf JumpBuffer;

std::map<std::string, int> fault_strings;
std::map<int, double> fault_probs;
std::map<int, int> fault_counts;
std::map<int, int> run_counts;
std::map<int, std::string> location_kinds;
std::map<int, unsigned long long> first_events;

int fault_cycle = 1;
std::string fault_mode = "int_msb";
bool fault_injected = false;
unsigned long long global_event_count = 0;
std::chrono::steady_clock::time_point pain_start;

extern int pain();

void finalize() {
  printf("%-12s\n", command);
  printf("\tsize\t%d\ttrials\t%d\n", array_size, num_trials);
  printf("\tsdc\t%d\tcore\t%d\n", num_sdc, num_core);
  exit(0);
}

void coredump(int index, int lb, int ub) {
  if (verbose) {
    printf("indexing fault %d not in %d-%d\n", index, lb, ub);
  }
  ++num_core;
  fprintf(stderr, "FI_RUN\tinjected=%d\ttotal_events=%llu\tstatus=index_fault\n",
          fault_injected ? 1 : 0, global_event_count);
  fflush(stderr);
  exit(2);
}

static const char help[] =
    "ADAD/ADAF deterministic single-fault driver\n\n"
    "  -l              list dynamically reached fault locations\n"
    "  -p <id,prob>    select a location (campaigns use probability 1)\n"
    "  -c <N>          inject on occurrence N of the location [1]\n"
    "  -t <mode>       int_msb, int_lsb, int_middle, float_mantissa_middle,\n"
    "                  float_exponent_middle, or boolean\n"
    "  -s <seed>       accepted for compatibility\n"
    "  -v              verbose\n"
    "  -h              help\n";

int main(int argc, char **argv) {
  int seed = -1;
  int list = 0;
  int id = 0;
  int c;
  double probability = 0.0;

  command = argv[0];
  while ((c = getopt(argc, argv, "n:m:p:s:c:t:lvh")) != -1) {
    switch (c) {
      case 'n':
        array_size = atoi(optarg);
        break;
      case 'm':
        num_trials = atoi(optarg);
        break;
      case 'p':
        if (sscanf(optarg, "%d,%lg", &id, &probability) != 2 ||
            probability < 0.0 || probability > 1.0) {
          fprintf(stderr, "bad -p argument %s\n", optarg);
          return 2;
        }
        fault_probs[id] = probability;
        break;
      case 's':
        seed = atoi(optarg);
        break;
      case 'c':
        fault_cycle = atoi(optarg);
        if (fault_cycle < 1) {
          fprintf(stderr, "occurrence must be at least 1\n");
          return 2;
        }
        break;
      case 't':
        fault_mode = optarg;
        break;
      case 'l':
        list = 1;
        break;
      case 'v':
        verbose = 1;
        break;
      case 'h':
      default:
        fputs(help, stderr);
        return c == 'h' ? 0 : 2;
    }
  }

  if (seed >= 0) {
    srandom(static_cast<unsigned int>(seed));
  }

  pain_start = std::chrono::steady_clock::now();
  int pain_status = pain();

  if (list) {
    printf("FI_LOCATIONS_BEGIN\n");
    printf("id\tkind\toperation\tcount\tfirst_event\tdescription\n");
    for (const auto &entry : fault_strings) {
      const int location_id = entry.second;
      const std::string &combined = entry.first;
      const size_t tab = combined.find('\t');
      const std::string operation = combined.substr(0, tab);
      const std::string description =
          tab == std::string::npos ? "N/A" : combined.substr(tab + 1);
      printf("%d\t%s\t%s\t%d\t%llu\t%s\n", location_id,
             location_kinds[location_id].c_str(), operation.c_str(),
             run_counts[location_id], first_events[location_id],
             description.c_str());
    }
    printf("FI_TOTAL_EVENTS\t%llu\n", global_event_count);
    printf("FI_LOCATIONS_END\n");
  }

  fprintf(stderr, "FI_RUN\tinjected=%d\ttotal_events=%llu\tstatus=normal\n",
          fault_injected ? 1 : 0, global_event_count);
  return pain_status;
}
