/* A simple program to test MCA API integration with STHORM PEs

   Ronan.Keryell@silkan.com
*/

#include "mca-api-common/smecy-mca-common.h"
/* Define the STHORM functions do deal with the fabbric: */
#include "mcapi_fabric_helper.h"
#include "STHORM_mapping.h"
/* Include the producer on the fabric part */
#include "mca-api-common/producer.h"

/* Entry point for STHORM MCAPI on the fabric.

   This is executed by a side effect of mcapi_initialize() from the host
   side.
*/
void mcapi_domain_entry() {
  MCAPI_TRACE_C("Entering fabric side");
  mcapi_status_t status;
  mcapi_domain_t domain_id = mcapi_domain_id_get(&status);
  MCAPI_CHECK_STATUS(status);

  if (MCAPI_SUCCESS == status)  {
    if (PRODUCER_DOMAIN == domain_id) {
       MCAPI_TRACE_CI("Creating the producer: domain_id = #", domain_id);
       create_node(PRODUCER_DOMAIN, producer, NULL, NULL, NULL);
    }
    else
      MCAPI_TRACE_CI("Fabric cluster not used: domain_id = #", domain_id);
  }
}
