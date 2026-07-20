#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#include <map>
#include <string>
#include <iostream>
#include "adad.h"

int   array_size    = 100;
int   num_trials    = 1;
int   current_trial = 0;
int   num_sdc       = 0;
int   num_core      = 0;
int   last_id       = -1;
int   verbose       = 0;

char *command       = NULL;
// the jump buffer defines the place in the loop we want to return to
jmp_buf JumpBuffer;

std::map<std::string, int> fault_strings;
std::map<int, double>      fault_probs;
std::map<int, int>         fault_counts, run_counts;
int fault_cycle=0, fault_type=0;

extern int pain();

void finalize() {
  std::map<int, double>::iterator it1;
  std::map<int, int>::iterator it2;

  printf("%-12s\n", command);
  printf("\tsize\t%d\ttrials\t%d\n", array_size, num_trials);
  printf("\tsdc\t%d\tcore\t%d\n", num_sdc, num_core);

  printf("Injection Fault_Probs:\n\tid\tprob\n");
  for (it1 = fault_probs.begin(); it1 != fault_probs.end(); it1++) {
    printf("\t%d\t%g\n", it1->first, it1->second);
  }

  printf("Fault Counts:\n");
  for (it2 = fault_counts.begin(); it2 != fault_counts.end(); it2++) {
    printf("\t%d\t%d\n", it2->first, it2->second);
  }
  exit(0);
}

// this function gets called when the indexing detects a core dump
void coredump(int index, int lb, int ub) {
  if (verbose) printf("run %4d/%4d had an indexing fault %d not in %d-%d\n", current_trial, num_trials, index, lb, ub);
  ++num_core;
  ++current_trial;
  // are we at the end?
  if (current_trial >= num_trials) finalize();
  // no, so we jump back into the loop
  longjmp(JumpBuffer, 1);
}

static char help[] = "\t\t\033[1;31mSORT\033[0m help\n\n\
  command line flags:\n\
    -l          do one run and print the ids\n\
    -n <N>	size of array [100]\n\
    -m <M>	number of monte-carlo trials [1]\n\
    -p <i,P>	probability of a fault at the specified id [0.0] (*)\n\
    -s <S>	specify random integer seed [use time()]\n\
    -v		be quite verbose [no, be quiet]\n\
    -h		print this help\n\
    -f <C>      fault type: 0: MANTISSA, 1: EXPONENT, 2: SIGN, 3: ANY [3] (*)\n\
 (*) more than one -p flag can be given\n";

int main(int argc, char **argv) {\
  int      SEED, LIST, ID, i;
  char	   c;
  double   prob;
  SEED = -1;
  LIST = 0;

  // stash the command name
  command = argv[0];

  // process command line options
  while ((c = getopt(argc, argv, "n:m:p:s:c:f:lvh")) != -1) {
    switch (c) {
    case 'n':
      array_size = atoi(optarg);
      break;
    case 'm':
      num_trials = atoi(optarg);
      break;
    case 'c':
      fault_cycle = atoi(optarg);
      break;
    case 's':
      i = atoi(optarg);
      if (i <= 0) {
        fprintf(stderr, "bad seed %s\n", optarg);
        exit(-1);
      } else {
        SEED = i;
      }
      fprintf(stderr,"SEED is set from input: %d \n", SEED);
      break;
    case 'p':
      if (sscanf(optarg, "%d,%lg", &ID, &prob) != 2) {
        fprintf(stderr, "bad -p argument %s\n", optarg);
        exit(-1);
      }
      if (prob < 0.0 || prob > 1.0) {
        fprintf(stderr, "bad -p argument %s\n", optarg);
        exit(-1);
      }
      fault_probs[ID] = prob;
      break;
    case 'l':
      LIST = 1;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'f':
      fault_type = atoi(optarg);
      if (fault_type < 0 || fault_type > 3) {
        fprintf(stderr, "bad -f argument %s\n", optarg);
        exit(-1);
      }
      break;
    case 'h':
    default:
      fprintf(stderr, "%s", help);
      exit(0);
    }
  }

  struct timeval tv;
  gettimeofday(&tv,NULL);
  //unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
  if (SEED == -1) SEED = tv.tv_usec;
  fprintf(stderr,"SEED=%d \n", SEED);
  srandom(SEED);
  fprintf(stderr, "SEED is set to: %d\n", SEED);

  if (LIST) {
    pain();

    // std::map<std::string, int>::iterator it0;
    // std::map<int, int>::iterator it2;
    // printf("Fault Locations:\n\tid\tdescription\n");
    // for (it0 = fault_strings.begin(); it0 != fault_strings.end(); it0++) {
    //   printf("\t%d\t%s\n", it0->second, it0->first.c_str());
    // }
    // printf("Run Counts:\n");
    // for (it2 = run_counts.begin(); it2 != run_counts.end(); it2++) {
    //   printf("\t%d\t%d\n", it2->first, it2->second);
    // }

    std::map<std::string, int>::iterator it0;
    printf("Fault Locations + Counts:\n\tid\tdescription\tcount\n");
    for (it0 = fault_strings.begin(); it0 != fault_strings.end(); it0++) {
        int id = it0->second;
        std::string desc = it0->first;

        int count = 0;
        auto itCount = run_counts.find(id);
        if (itCount != run_counts.end()) {
            count = itCount->second;
        }
        printf("\t%d\t%s\t%d\n", id, desc.c_str(), count);
    }
    return(0);
  }

  pain();
  //finalize();
  return 0;
}
