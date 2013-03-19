/* A simple program to test MCA API integration with STHORM PEs

   Ronan.Keryell@silkan.com
*/

#include "mca-api-common/smecy-mca-common.h"
#include "mcapi_trace_helper.h"
#include "STHORM_mapping.h"
/* Include the consumer on the host, the producer will be on the fabric
   part */
#include "mca-api-common/consumer.h"

int main (int argc, char *argv[]) {
  /* Increase the level to trace down MCA things */
  //mcapi_set_debug_level(0);

  MCAPI_TRACE_C("Entering host side");
  /* Keep consumer() in the current task and by a side effect launch the
     fabric */
  MCAPI_TRACE_C("Initializing consumer task");
  consumer((mcapi_param_t *) &argv[1]);

  return 0;
}
