/* A simple program to test MCA API integration with OpenMP 3 tasks
   instead of hard-core pthreads

   Ronan.Keryell@wild-systems.com
*/


#include <stdio.h>
/* For main() exit codes */
#include <stdlib.h>

/* To use MCAPI from the MultiCore Association */
#include<mcapi.h>
#include<mca.h>

enum {
  SMECY_DOMAIN = 42,
  PRODUCER_NODE = 3,
  CONSUMER_NODE = 7,
  SEND_PORT = 11,
  RECEIVE_PORT = 13
};

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
void MCAPI_check_status(mcapi_status_t status,
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

void producer() {
  //mcapi_node_attributes_t node_attributes;
  mcapi_param_t parameters;
  mcapi_info_t info;
  mcapi_endpoint_t data_transmit_endpoint;
  mcapi_endpoint_t data_receive_endpoint;

  mcapi_status_t status;

#ifdef SMECY_VERBOSE
    fputs("Entering the producer\n", stderr);
#endif

  // init node attributes
  //mcapi_node_init_attributes(&node_attributes, &status);
  //MCAPI_CHECK_STATUS(status);
  //mcapi_initialize(SMECY_DOMAIN, PRODUCER_NODE, &node_attributes,
  //		   &parameters, &info, &status);
  mcapi_initialize(SMECY_DOMAIN, PRODUCER_NODE,
		   &parameters, &info, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_endpoint_t send = mcapi_endpoint_create(SEND_PORT, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_endpoint_t receive = mcapi_endpoint_get(SMECY_DOMAIN,
						CONSUMER_NODE,
						RECEIVE_PORT,
						MCA_INFINITE, &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < 100; i++) {
    char message[100];
    snprintf(message, sizeof(message), "Salut %d", i);
    mcapi_msg_send(send, receive, message, sizeof(message),
		   MCAPI_MAX_PRORITY, &status);
    MCAPI_CHECK_STATUS(status);
  }

  mcapi_finalize(&status);
  MCAPI_CHECK_STATUS(status);

#ifdef SMECY_VERBOSE
    fputs("Exiting the producer\n", stderr);
#endif

  //for(;;) {
    //printf("Produce\n");
  //}

}


void consumer() {
#ifdef SMECY_VERBOSE
    fputs("Entering the consumer\n", stderr);
#endif
  mcapi_param_t parameters;
  mcapi_info_t info;
  mcapi_endpoint_t data_transmit_endpoint;
  mcapi_endpoint_t data_receive_endpoint;

  mcapi_status_t status;
  mcapi_initialize(SMECY_DOMAIN, CONSUMER_NODE,
		   &parameters, &info, &status);
  MCAPI_CHECK_STATUS(status);

  mcapi_endpoint_t receive = mcapi_endpoint_create(RECEIVE_PORT, &status);
  MCAPI_CHECK_STATUS(status);

  for (int i = 0; i < 100; i++) {
    char buffer[100];
    size_t received_size;
    mcapi_msg_recv(receive, buffer, sizeof(buffer), &received_size, &status);
    MCAPI_CHECK_STATUS(status);
    /* Avoid later over-reading: */
    buffer[sizeof(buffer) - 1] = '\0';
    printf("Received %zd characters: %s\n", received_size, buffer);
  }

  mcapi_finalize(&status);
  MCAPI_CHECK_STATUS(status);
#ifdef SMECY_VERBOSE
    fputs("Exiting the consumer\n", stderr);
#endif
}


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
