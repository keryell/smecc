#include <stdio.h>
/* For main() exit codes */
#include <stdlib.h>

/* To use MCAPI from the MultiCore Association */
#include<mcapi.h>
#include<mca.h>

#ifdef MCAPI_STHORM
/* Use the STHORM tracing API... */
#include "mcapi_trace_helper.h"
#else
/* ...or not */
#define MCAPI_TRACE(args, ...)
#define MCAPI_TRACE_C(trace)
#define MCAPI_TRACE_CI(trace,val)
#define MCAPI_TRACE_CF(trace,val)
#define MCAPI_TRACE_CC(trace,msg)
#define MCAPI_TRACE_CP(trace,add)
#define MCAPI_TRACE_CS(trace,status)
#endif

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
#ifndef MCAPI_MAX_STATUS_SIZE
#define MCAPI_MAX_STATUS_SIZE 250
#endif
#ifdef MCAPI_STHORM
    char *message = "";
#else
    char message[MCAPI_MAX_STATUS_SIZE];
#endif
    mcapi_display_status(status, message, MCAPI_MAX_STATUS_SIZE);
    fprintf(stderr,"API call fails in file '%s', function '%s',"
	    " line %d with error:\n\t%s\n",
	    file, function, line, message);
#ifdef MCAPI_STHORM
    /* Rely on the STHORM to display the status */
    MCAPI_TRACE_CS("", status);
#endif
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
