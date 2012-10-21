/* A simple program to test MCA API integration with OpenMP 3 tasks
   instead of hard-core pthreads

   Ronan.Keryell@wild-systems.com
*/

#include "mca-api-common/smecy-mca-common.h"


enum {
  SMECY_DOMAIN = 42,
  PRODUCER_NODE = 3,
  CONSUMER_NODE = 7,
  SEND_MSG_PORT = 11,
  RECEIVE_MSG_PORT = 13,
  SEND_PKT_PORT = 41,
  RECEIVE_PKT_PORT = 43,
  N_MSG = 10
};

#include "mca-api-common/producer.h"

#include "mca-api-common/consumer.h"


int main() {
  /* Increase the level to trace down MCA things */
  mcapi_set_debug_level(0);

  /* Launch 2 threads */
#pragma omp parallel num_threads(2)
  {
    /* But only one can run now. Useless to wait at the end of single
       section since there is the join at the end of the parallel
       section */
#pragma omp single nowait
    {
      /* Execute producer() in a new task */
#pragma omp task
      producer();
      /* We can keep consumer() in the current task, so no need for the
	 following pragma */
      //#pragma omp task
      consumer();
    }
  }
  return EXIT_SUCCESS;
}
