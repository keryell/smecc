/* SMECY low-level runtime implementation with MCAPI

   Ronan.Keryell@silkan.com
*/

/* For the final pause() */
#include <unistd.h>
/* For malloc() */
#include <stdlib.h>
/* For memcpy() */
#include <string.h>
/* For... assert() :-) */
#include <assert.h>

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
void static SMECY_MCAPI_check_status(mcapi_status_t status,
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


/* The wrapping macro for MCAPI_check_status to capture the call site
   information */
#define SMECY_MCAPI_CHECK_STATUS(status)                          \
  SMECY_MCAPI_check_status(status, __FILE__, __func__, __LINE__)




/* SMECY_IMP_ are the real implementations doing the real work, to be
   defined somewhere else. */

/* Implementation macros to deal with mapping and function executions */


// Create a variable name used to pass an argument to function
#define SMECY_IMP_VAR_ARG(func, arg, pe, ...)	\
  SMECY_CONCATN(SMECY_CONCATN(p4a_##pe##_,SMECY_CONCATENATE(__VA_ARGS__)),_##func##_##arg)

//  SMECY_CONCAT(SMECY_CONCAT(p4a_##pe##_,SMECY_CONCATENATE(__VA_ARGS__)),##_##func##_##arg)

/* Wrapper that can be used for example to launch the function in another
   thread */
#define SMECY_IMP_LAUNCH_WRAPPER(func_call) func_call


// Implementations for the SMECY library on MCAPI


/* Create a packet channel for reception listening on receive_port */
mcapi_pktchan_recv_hndl_t static
SMECY_MCAPI_receive_gate_create(mcapi_port_t receive_port) {
  mcapi_status_t status;
  /* Create the local endpoint for reception */
  mcapi_endpoint_t pkt_receive = mcapi_endpoint_create(receive_port,
						       &status);
  SMECY_MCAPI_CHECK_STATUS(status);

  mcapi_pktchan_recv_hndl_t receive_gate;
  mcapi_request_t handle;
  /* Let the sender do the connection and open for receive */
  mcapi_pktchan_recv_open_i(&receive_gate, pkt_receive, &handle, &status);
  SMECY_MCAPI_CHECK_STATUS(status);
  size_t size;
  /* Wait for the completion of opening */
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS(status);
  return receive_gate;
}

/* Create a packet channel for transmission */
mcapi_pktchan_send_hndl_t static
SMECY_MCAPI_send_gate_create(mcapi_port_t send_port,
                             mcapi_domain_t receive_domain,
                             mcapi_node_t receive_node,
                             mcapi_port_t receive_port) {
  mcapi_status_t status;
  /* Create the local port to send the data */
  mcapi_endpoint_t pkt_send = mcapi_endpoint_create(send_port, &status);
  SMECY_MCAPI_CHECK_STATUS(status);

  /* Get the remote end point. Wait if it is not created at the receive
     side */
  mcapi_endpoint_t pkt_receive = mcapi_endpoint_get(receive_domain,
						    receive_node,
						    receive_port,
						    MCA_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS(status);

  mcapi_request_t handle;
  /* Start a connection request... */
  mcapi_pktchan_connect_i(pkt_send, pkt_receive, &handle, &status);
  SMECY_MCAPI_CHECK_STATUS(status);
  /* ...and wait for its completion */
  size_t size;
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS(status);

  mcapi_pktchan_send_hndl_t send_gate;
  /* Open this side of the channel for sending... */
  mcapi_pktchan_send_open_i(&send_gate, pkt_send, &handle, &status);
  SMECY_MCAPI_CHECK_STATUS(status);
  /* And wait for the opening */
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS(status);
  return send_gate;
}

enum {
  /* The localization of the host inside the MCAPI realm */
  SMECY_MCAPI_HOST_DOMAIN = 5,
  SMECY_MCAPI_HOST_NODE = 0,
  /* The port numbers to connect the host and the PE */
  SMECY_MCAPI_HOST_TX_PORT = 1,
  SMECY_MCAPI_HOST_RX_PORT = 2,
  SMECY_MCAPI_PE_TX_PORT = 3,
  SMECY_MCAPI_PE_RX_PORT = 4,
};

/* Analyze the PE type and coordinates */
#define SMECY_MCAPI_PARSE_PE(pe, ...)                                   \
  /* Redirect to the function that knows about pe accelerator */        \
  SMECY_MCAPI_PARSE_PE_##pe(__VA_ARGS__)


/* Analyze the STHORM coordinates: domain and node numbers */
#define SMECY_MCAPI_PARSE_PE_STHORM(d, n)       \
  mcapi_domain_t domain = d;/*
                             */                 \
  mcapi_node_t node = n;

#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_set(func, pe, ...)                                    \
  SMECY_LBRACE /* To have local variable
                */                                                      \
  mcapi_status_t SMECY_MCAPI_status; /*
  Analyze the PE type and coordinates into domain & node */             \
  SMECY_MCAPI_PARSE_PE(pe, __VA_ARGS__);                                \
  /* Since the host part may be used in a loop, only set-up the         \
     connection once by testing this flag */                            \
  static char already_initialized = 0; /*
                                        */                              \
  static mcapi_pktchan_send_hndl_t transmit = -1; /*
                                                   */                   \
  static mcapi_pktchan_recv_hndl_t receive = -1;  /*
                                                   */                   \
  if (!already_initialized) { /*
                                Do it in this order compared with the PE
                                to avoid dead-locks on opening: first open
                                a connection to send data to the PE */  \
    transmit = SMECY_MCAPI_send_gate_create(SMECY_MCAPI_HOST_TX_PORT,   \
                                            domain,                     \
                                            node,                       \
                                            SMECY_MCAPI_PE_RX_PORT); /*
                  Then open a connection to receive data from the PE */ \
    receive = SMECY_MCAPI_receive_gate_create(SMECY_MCAPI_HOST_RX_PORT);/*
                                                                         */ \
  }
#else
/* This is on the accelerator side */
#define SMECY_IMP_set(func, pe, ...)                                    \
  SMECY_LBRACE /* <- '{' To have local variables
                */                                                      \
  mcapi_status_t SMECY_MCAPI_status;    /* Do it in this order compared
     with the host to avoid dead-locks on opening: first wait from the
     host a connection to receive data to the PE
                                        */                              \
  mcapi_pktchan_recv_hndl_t receive =                                   \
    SMECY_MCAPI_receive_gate_create(SMECY_MCAPI_PE_RX_ENDP); /*
                                                              */        \
  mcapi_pktchan_send_hndl_t transmit =                                  \
    SMECY_MCAPI_send_gate_create(SMECY_MCAPI_PE_TX_PORT,                \
                                 SMECY_MCAPI_HOST_DOMAIN,               \
                                 SMECY_MCAPI_HOST_NODE,                 \
                                 SMECY_MCAPI_HOST_RX_PORT);             \
  /* Enter the infinite service loop on the PE
   */                                                                   \
  for(;;) SMECY_LBRACE
#endif


#define SMECY_IMP_accelerator_end(func, pe, ...)        \
  /* End of the accelerated part */                     \
  SMECY_RBRACE


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_send_arg(func, arg, type, value, pe, ...)             \
  /* Use an intermediate variable to be able to have an address on it
     if literal is given */                                             \
  type SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = value;           \
  /* Send the scalar data to the PE
   */                                                                   \
  mcapi_pktchan_send(transmit,                                          \
                     &SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__),    \
                     sizeof(type),                                      \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);
#else
/* This is on the accelerator side */
#define SMECY_IMP_send_arg(func, arg, type, value, pe, ...)             \
  /* A pointer that will point to the received message */               \
  char *SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__)_message;          \
  /* The size of the received scalar received
   */                                                                   \
  size_t received_size;                                                 \
  /* Receive the packet with the value
   */                                                                   \
  mcapi_pktchan_recv(receive, (void **)&SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__)_message, \
                     &received_size, &SMECY_MCAPI_status);              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(status);                                     \
  /* Store the value to the argument to be given to the function
     call */                                                            \
  type SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = *(type)value;
#endif

#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_cleanup_send_arg(func, arg, type, value, pe, ...)     \
/* Nothing to do for SMECY__cleanup_send_arg */
#else
/* This is on the accelerator side */
#define SMECY_IMP_cleanup_send_arg(func, arg, type, value, pe, ...)     \
  /* Give back the memory buffer to the API for recycling
   */                                                                   \
  mcapi_pktchan_release((void **)&SMECY_IMP_VAR_ARG(func,               \
                                                    arg,                \
                                                    pe,                 \
                                                    __VA_ARGS__)_message,\
                        &SMECY_MCAPI_status);                           \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(status);
#endif

#define SMECY_IMP_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  type* SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = addr

#define SMECY_IMP_cleanup_send_arg_vector(func, arg, type, addr, size, pe, ...)

#define SMECY_IMP_update_arg_vector(func, arg, type, addr, size, pe, ...) \
  type* SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = addr

#define SMECY_IMP_cleanup_update_arg_vector(func, arg, type, addr, size, pe, ...)

#define SMECY_IMP_launch(func, n_args, pe, ...)    \
  SMECY_IMP_launch_##n_args(func, pe, __VA_ARGS__)

#define SMECY_IMP_prepare_get_arg_vector(func, arg, type, addr, size, pe, ...) \
  type* SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = addr

#define SMECY_IMP_get_arg_vector(func, arg, type, addr, size, pe, ...)

/* TODO: To be implemented... */
#define SMECY_IMP_get_return(func, type, pe, ...)

/* Implementation of the function calls themselves */

/* Call a function without parameter */
#define SMECY_IMP_launch_0(func, pe, ...)  \
  SMECY_IMP_LAUNCH_WRAPPER(func())
/* For the recurrence afterwards: no parameter when 0 parameter :-) */
#define SMECY_IMP_ARG_launch_0(func, pe, ...)

/* Call a function with 1 parameter */
#define SMECY_IMP_ARG_launch_1(func, pe, ...) SMECY_IMP_VAR_ARG(func, 1, pe, __VA_ARGS__)
#define SMECY_IMP_launch_1(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_1(func, pe, __VA_ARGS__)))

/* Declare the launchers for function calls with 2 and more parameters by
   recurrence. Each time, add a parameter to the list of parameter of the
   previous call */
#define SMECY_IMP_ARG_launch_2(func, pe, ...) SMECY_IMP_ARG_launch_1(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 2, pe, __VA_ARGS__)
#define SMECY_IMP_launch_2(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_2(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_3(func, pe, ...) SMECY_IMP_ARG_launch_2(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 3, pe, __VA_ARGS__)
#define SMECY_IMP_launch_3(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_3(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_4(func, pe, ...) SMECY_IMP_ARG_launch_3(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 4, pe, __VA_ARGS__)
#define SMECY_IMP_launch_4(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_4(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_5(func, pe, ...) SMECY_IMP_ARG_launch_4(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 5, pe, __VA_ARGS__)
#define SMECY_IMP_launch_5(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_5(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_6(func, pe, ...) SMECY_IMP_ARG_launch_5(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 6, pe, __VA_ARGS__)
#define SMECY_IMP_launch_6(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_6(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_7(func, pe, ...) SMECY_IMP_ARG_launch_6(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 7, pe, __VA_ARGS__)
#define SMECY_IMP_launch_7(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_7(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_8(func, pe, ...) SMECY_IMP_ARG_launch_7(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 8, pe, __VA_ARGS__)
#define SMECY_IMP_launch_8(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_8(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_9(func, pe, ...) SMECY_IMP_ARG_launch_8(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 9, pe, __VA_ARGS__)
#define SMECY_IMP_launch_9(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_9(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_10(func, pe, ...) SMECY_IMP_ARG_launch_9(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 10, pe, __VA_ARGS__)
#define SMECY_IMP_launch_10(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_10(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_11(func, pe, ...) SMECY_IMP_ARG_launch_10(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 11, pe, __VA_ARGS__)
#define SMECY_IMP_launch_11(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_11(func, pe, __VA_ARGS__)))

#define SMECY_IMP_ARG_launch_12(func, pe, ...) SMECY_IMP_ARG_launch_11(func, pe, __VA_ARGS__),SMECY_IMP_VAR_ARG(func, 12, pe, __VA_ARGS__)
#define SMECY_IMP_launch_12(func, pe, ...)                               \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_ARG_launch_12(func, pe, __VA_ARGS__)))

/* Implementation macros to deal with streaming */

/* Not implemented yet in MCAPI */
