/* -*- mode: c; c-basic-offset: 2; tab-width: 2 -*- */

/* Simple test case with producer/consumer in 2 threads

   Ronan.Keryell@silkan.com
*/

#include <stdio.h>

#include "../conditional_variable_openmp.h"


static int v = 0;

void producer(conditional_variable_t* nw) {
  for(;;) {
    COND_VAR_NOTIFY_WITH_OP(nw, {
  // Increment a global variable inside the critical section
  v++;
  // Make sure the value is globally visible
  _Pragma("omp flush(v)");
      });
  }
}


void consumer(conditional_variable_t* nw) {
  for(;;) {
    COND_VAR_WAIT_WITH_OP(nw, {
  // Make sure we can get the last version of this global variable
  _Pragma("omp flush(v)");
  // Increment a global variable inside the critical section
  printf("%d ", v);
  fflush(stdout);
      });
  }
}


void main() {
  conditional_variable_t nw;
  conditional_variable_init(&nw);

  // Create 2 threads
#pragma omp parallel num_threads(2)
  {
#pragma omp sections
    {
      // Call the producer in the first thread
      producer(&nw);
#pragma omp section
      consumer(&nw);
      // Call the consumer in the second thread
    }
  }
}
