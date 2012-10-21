#include <stdio.h>
/* For main() exit codes */
#include <stdlib.h>

/* To use MCAPI from the MultiCore Association */
#include<mcapi.h>
#include<mca.h>


/* Useful for debugging :

void mcapi_set_debug_level (int d) {
  mca_set_debug_level (d);
}

void mcapi_display_state (void* handle) {
  mcapi_trans_display_state(handle);
}

*/

/* Test error code.

   Display and exit on failure
*/
void static MCAPI_check_status(mcapi_status_t status,
			char file[],
			const char function[],
			int line) {
  if (status != MCAPI_SUCCESS) {
    /* Something went wrong */
#ifdef SMECY_VERBOSE
    /* Use a function from Linux MCAPI implementation to get a string
       translation of the status: */
    char message[MCAPI_MAX_STATUS_SIZE];
    mcapi_display_status(status, message, MCAPI_MAX_STATUS_SIZE);
    fprintf(stderr,"API call fails in file '%s', function '%s',"
	    " line %d with error:\n\t%s\n",
	    file, function, line, message);
#endif
    /* Exit and forward the error code to the OS: */
    exit(status);
  }
  /* Go on, no error */
  return;
}


/* The wrapping macro for MCAPI_check_status to get the call site
   information */
#define MCAPI_CHECK_STATUS(status)		\
  MCAPI_check_status(status, __FILE__, __func__, __LINE__)
