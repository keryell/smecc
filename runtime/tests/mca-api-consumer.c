/* A simple program to test MCA API integration with OpenMP 3 tasks
   instead of hard-core pthreads

   Ronan.Keryell@wild-systems.com
*/

#include "mca-api-common/smecy-mca-common.h"

#include "mca-api-common/multi-process-producer-comsumer.h"
#include "mca-api-common/consumer.h"

int main() {
  /* Increase the level to trace down MCA things */
  mcapi_set_debug_level(0);

  consumer();
  return EXIT_SUCCESS;
}
