/* SMECY low-level runtime implementation with MCAPI

   Ronan.Keryell@silkan.com
*/

/* For the final pause() */
#include <unistd.h>
/* For malloc() and atexit() */
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


enum {
  /* The STHORM geometry */
  SMECY_CLUSTER_NB = 4,
  SMECY_PE_NB = 32,
  /* The localization of the host inside the MCAPI realm */
  SMECY_MCAPI_HOST_DOMAIN = 5,
  SMECY_MCAPI_HOST_NODE = 0,
  /* The port numbers to connect the host and the PE */
  SMECY_MCAPI_HOST_TX_PORT = 1,
  SMECY_MCAPI_HOST_RX_PORT = 2,
  SMECY_MCAPI_PE_TX_PORT = 3,
  SMECY_MCAPI_PE_RX_PORT = 4,
};


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


// Create a unique variable name used to pass an argument to function
#define SMECY_IMP_VAR_ARG(func, arg, pe, ...)	\
  SMECY_CONCATN(SMECY_CONCATN(p4a_##pe##_,SMECY_CONCATENATE(__VA_ARGS__)),_##func##_##arg)

// Create a unique variable name for a message
#define SMECY_IMP_VAR_MSG(func, arg, pe, ...)	\
  SMECY_CONCATN(SMECY_CONCATN(p4a_##pe##_,SMECY_CONCATENATE(__VA_ARGS__)),_##func##_##arg##_msg)

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

/* Analyze the PE type and coordinates by redirect to the function that
   knows about the "pe" accelerator */
#define SMECY_MCAPI_PARSE_PE(pe, ...)                                   \
  SMECY_MCAPI_PARSE_PE_##pe(__VA_ARGS__)


/* Analyze the STHORM coordinates: domain and node numbers */
#define SMECY_MCAPI_PARSE_PE_STHORM(d, n)       \
  mcapi_domain_t domain = d;/*
                             */                 \
  mcapi_node_t node = n

/* Analyze the Host coordinates, which are not specified in the pragma.
   Replace them with the host MCAPI node */
#define SMECY_MCAPI_PARSE_PE_Host()                     \
  mcapi_domain_t domain = SMECY_MCAPI_HOST_DOMAIN;/*
                                                   */   \
  mcapi_node_t node = SMECY_MCAPI_HOST_NODE


static void SMECY_IMP_finalize() {
  mcapi_status_t status;
  /* Release the API use */
  mcapi_finalize(&status);
  SMECY_MCAPI_CHECK_STATUS(status);
}

static void SMECY_IMP_initialize_then_finalize() {
  //mcapi_node_attributes_t node_attributes;
  mcapi_param_t parameters;
  mcapi_info_t info;
  mcapi_status_t status;

  // init node attributes. Not clear in which MCAPI version it is needed...
  //mcapi_node_init_attributes(&node_attributes, &status);
  //MCAPI_CHECK_STATUS(status);
  //mcapi_initialize(SMECY_DOMAIN, PRODUCER_NODE, &node_attributes,
  //		   &parameters, &info, &status);
  mcapi_initialize(SMECY_MCAPI_HOST_DOMAIN, SMECY_MCAPI_HOST_NODE,
		   &parameters, &info, &status);
  SMECY_MCAPI_CHECK_STATUS(status);

  /* And the register the finalization for an execution at the end of the
     program execution */
  atexit(SMECY_IMP_finalize);
}


#ifdef SMECY_MCAPI_HOST
/* Open some MCAPI connections with the requested node

   The current implementation does not work if called many times with
   different PE because it caches the connection.
 */
#define SMECY_IMP_set(func, instance, pe, ...)                          \
  SMECY_LBRACE /* To have local variable
                */                                                      \
  mcapi_status_t SMECY_MCAPI_status; /*
  Analyze the PE type and coordinates into domain & node */             \
  SMECY_MCAPI_PARSE_PE(pe, __VA_ARGS__);                                \
  /* The handle to sending packets
   */                                                                   \
  static mcapi_pktchan_send_hndl_t P4A_transmit = -1;                   \
  /* The handle to receiving packets
   */                                                                   \
  static mcapi_pktchan_recv_hndl_t P4A_receive = -1;                    \
  /* Since the host part may be used in a loop, only set-up the         \
     connection once by testing this flag */                            \
  static char already_initialized = 0; /*
                                        */                              \
  if (!already_initialized) { /*
                                Do it in this order compared with the PE
                                to avoid dead-locks on opening: first open
                                a connection to send data to the PE */  \
    P4A_transmit = SMECY_MCAPI_send_gate_create(SMECY_MCAPI_HOST_TX_PORT, \
                                                domain,                 \
                                                node,                   \
                                                SMECY_MCAPI_PE_RX_PORT); /*
                  Then open a connection to receive data from the PE */ \
    P4A_receive = SMECY_MCAPI_receive_gate_create(SMECY_MCAPI_HOST_RX_PORT);/*
                                                                         */ \
  }                                                                     \
  /* Send the function name to run to the remode dispatcher,
     including the final '\0' */
  size_t length = strlen(#func) + 1;                                    \
  mcapi_pktchan_send(P4A_transmit,                                      \
                     #func,                                             \
                     length,                                            \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);
  /* The size of some received data
   */                                                                   \
  size_t P4A_received_size
#else
/* This is on the accelerator side, directed here by the dispatcher.

   If it is necessary to call this PE at the same time from different
   caller threads, one need to use different ports...
*/
#define SMECY_IMP_set(func, instance, pe, ...)                          \
  SMECY_LBRACE /* <- '{' To have local variables
                */                                                      \
    mcapi_status_t SMECY_MCAPI_status;    /* Do it in this order compared
      with the host to avoid dead-locks on opening: first wait from the
      host a connection to receive data to the PE
                                        */                              \
    size_t P4A_received_size
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_accelerator_end(func, instance, pe, ...)        \
            /* End of the accelerated part
             */                                 \
  SMECY_RBRACE
#else
/* This is on the accelerator side */
#define SMECY_IMP_accelerator_end(func, instance, pe, ...)        \
  SMECY_RBRACE                                                    \
  /* End of the accelerated part: go back to the dispatcher */
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_send_arg(func, arg, type, value, pe, ...)             \
  /* Use an intermediate variable to be able to have an address on it
     if literal is given */                                             \
  type SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = value;           \
  /* Send the scalar data to the PE
   */                                                                   \
  mcapi_pktchan_send(P4A_transmit,                                        \
                     &SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__),    \
                     sizeof(type),                                      \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status)
#else
/* This is on the accelerator side */
#define SMECY_IMP_send_arg(func, arg, type, value, pe, ...)             \
  /* A pointer that will point to the received message */               \
  type* SMECY_IMP_VAR_MSG(func, arg, pe, __VA_ARGS__);                  \
  /* Receive the packet with the value
   */                                                                   \
  mcapi_pktchan_recv(P4A_receive,                                       \
                     (void **)&SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__), \
                     &P4A_received_size,                                \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);                        \
  /* Store the value in the argument to be given to the function
     call */                                                            \
  type SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) =                  \
    *SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__)
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_cleanup_send_arg(func, arg, type, value, pe, ...)     \
/* Nothing to do for SMECY_cleanup_send_arg */
#else
/* This is on the accelerator side */
#define SMECY_IMP_cleanup_send_arg(func, arg, type, value, pe, ...)     \
  /* Give back the memory buffer to the API for recycling
   */                                                                   \
  mcapi_pktchan_release(SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__),     \
                        &SMECY_MCAPI_status);                           \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status)
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  /* Send the vector data to the PE
   */                                                                   \
  mcapi_pktchan_send(P4A_transmit, addr, size, &SMECY_MCAPI_status);    \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status)
#else
/* This is on the accelerator side */
#define SMECY_IMP_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  /* A pointer that will point to the received message
   */                                                                   \
  type* SMECY_IMP_VAR_MSG(func, arg, pe, __VA_ARGS__);                  \
  /* Receive the packet with the value
   */                                                                   \
  mcapi_pktchan_recv(P4A_receive,                                       \
                     (void **)&SMECY_IMP_VAR_MSG(func, arg, pe, __VA_ARGS__), \
                     &P4A_received_size,                                \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);                         \
  /* Store the address if the vector into the argument to be given
     to the function call */                                            \
  type* SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) =                 \
    SMECY_IMP_VAR_MSG(func, arg, pe, __VA_ARGS__)
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_cleanup_send_arg_vector(func, arg, type, addr, size, pe, ...)
/* Nothing to do for SMECY_cleanup_send_arg_vector */
#else
/* This is on the accelerator side */
#define SMECY_IMP_cleanup_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  /* Give back the memory buffer to the API for recycling
   */                                                                   \
  mcapi_pktchan_release(SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__),     \
                        &SMECY_MCAPI_status);                           \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status)
#endif


#define SMECY_IMP_update_arg_vector(func, arg, type, addr, size, pe, ...) \
  TODO_SMECY_IMP_update_arg_vector


#define SMECY_IMP_cleanup_update_arg_vector(func, arg, type, addr, size, pe, ...) \
  TODO_SMECY_IMP_cleanup_update_arg_vector


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_launch(func, n_args, pe, ...)    \
  /* Nothing to launch: it is done on the accelerator side */
#else
/* This is on the accelerator side */
#define SMECY_IMP_launch(func, n_args, pe, ...)    \
  SMECY_IMP_launch_##n_args(func, pe, __VA_ARGS__)
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_prepare_get_arg_vector(func, arg, type, addr, size, pe, ...) \
  /* The pointer to the packet received from the PE by MCAPI
   */                                                           \
  type* SMECY_IMP_VAR_MSG(func, arg, pe, __VA_ARGS__)
#else
/* This is on the accelerator side */
#define SMECY_IMP_prepare_get_arg_vector(func, arg, type, addr, size, pe, ...) \
  /* Allocate the memory given to the function to receive the data
     from the execution */                                      \
  type SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__)[size]
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_get_arg_vector(func, arg, type, addr, size, pe, ...)  \
  /* Receive the vector result from the accelerator
   */                                                                   \
  mcapi_pktchan_recv(P4A_receive,                                       \
                     (void **)&SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__), \
                     &P4A_received_size,                                \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);                         \
  /* Store the received vector into the destination vector
   */                                                                   \
  memcpy(addr,                                                          \
         SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__),                    \
         size*sizeof(type));                                            \
  /* Give back the memory buffer to the API for recycling
   */                                                                   \
  mcapi_pktchan_release(SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__),     \
                        &SMECY_MCAPI_status);                           \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status)
#else
/* This is on the accelerator side */
#define SMECY_IMP_get_arg_vector(func, arg, type, addr, size, pe, ...)  \
  /* Send the vector data given by the function execution back to the host
   */                                                                   \
  mcapi_pktchan_send(P4A_transmit,                                      \
                     SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__),     \
                     size*sizeof(type),                                 \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status)
#endif



/* TODO: To be implemented... */
#define SMECY_IMP_get_return(func, type, pe, ...) \
  TODO_SMECY_IMP_get_return


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


/* Dispatching code */

/* Initialize MCAPI on an accelerator node.
*/
void SMECY_init_mcapi_node(int smecy_cluster, int smecy_pe) {
  mcapi_param_t parameters;
  mcapi_info_t info;

  mcapi_status_t status;
  mcapi_initialize(SMECY_MCAPI_HOST_DOMAIN, SMECY_MCAPI_HOST_NODE,
		   &parameters, &info, &status);
  SMECY_MCAPI_CHECK_STATUS(status);
}

#define SMECY_begin_accel_function_dispatch                             \
  /* The dispatch function to be run on a PE
   */                                                                   \
  void SMECY_accel_function_dispatch(int smecy_cluster, int smecy_pe) SMECY_LBRACE \
    /* Create the channels to communicate with the host using global
       variables
     */                                                                   \
    mcapi_pktchan_send_hndl_t P4A_receive =                               \
      SMECY_MCAPI_receive_gate_create(SMECY_MCAPI_PE_RX_PORT); /*         \
                                                                */        \
    mcapi_pktchan_send_hndl_t P4A_transmit =                              \
      SMECY_MCAPI_send_gate_create(SMECY_MCAPI_PE_TX_PORT,                \
                                   SMECY_MCAPI_HOST_DOMAIN,               \
                                   SMECY_MCAPI_HOST_NODE,                 \
                                   SMECY_MCAPI_HOST_RX_PORT);             \
    /* Enter the infinite service loop on the PE
     */                                                                   \
    for(;;) SMECY_LBRACE                                                  \
      SMECY_PRINT_VERBOSE("PE %d %d is waiting for a job\n",              \
                           smecy_cluster, smecy_pe)                       \
      /* Wait for the function name to run:
       */                                                                 \
      char *function_name;                                                \
      size_t P4A_received_size;                                           \
      mcapi_status_t SMECY_MCAPI_status;                                  \
      mcapi_pktchan_recv(P4A_receive,                                     \
                        (void **)&function_name,                          \
                        &P4A_received_size,                               \
                        &SMECY_MCAPI_status);                             \
      /* Check the correct execution
       */                                                                 \
      SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);                       \
      do SMECY_LBRACE                                                     \
        /* Find the right function to run
         */


#define SMECY_end_accel_function_dispatch                               \
        /* If we get here, we did not encounter a break and so nothing  \
           matches the requested function */                            \
        fprintf(stderr,"No candidate function to execute \"%s\" on PE %d %d\n", \
                function_name, smecy_cluster, smecy_pe);                \
        exit(-1);                                                       \
      SMECY_RBRACE while (0);                                           \
      mcapi_pktchan_release(function_name,                              \
                            &SMECY_MCAPI_status);                       \
      /* Check the correct execution                                    \
       */                                                               \
      SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);                     \
      /* End of the PE accelerator polling loop.                        \
       */                                                               \
    SMECY_RBRACE                                                        \
  /* There is no finalize because of the infinite loop in               \
     smecy_accel_function_dispatch                                      \
  */                                                                    \
  SMECY_RBRACE


#define SMECY_dispatch_accel_func(function, instance)                   \
  /* If we receive a message to activate this function
     launch it!

     Of course in a final implementation, do not use this linear search
     with string comparisons...
  */                                                                    \
  if (strcmp(function_name, #function) == 0) {                          \
    /* Call the accelerator function without any parameter
       since the parameter as indeed used as only local variables
       inside
    */                                                                  \
    SMECY_PRINT_VERBOSE("PE %d %d is executing instance " #instance     \
                        " of function \"" #function "\"\n",             \
                        smecy_cluster, smecy_pe)                        \
    smecy_accel_##function##_##instance(P4A_transmit, P4A_receive);     \
    /* Wait for next job to do
     */                                                                 \
    break;                                                              \
  }

#ifdef SMECY_MCAPI_HOST
/* Main function that start all the MCAPI threads that runs on the
   accelerator */
#define SMECY_start_PEs_dispatch /* Nothing since we inline the main */
/* On the host, wrap the old main into a new one to start MCAPI before
   and stop it after */
int main(int argc, char *argv[]) {
  SMECY_init_mcapi_node(SMECY_MCAPI_HOST_DOMAIN, SMECY_MCAPI_HOST_NODE);
  int error_code = smecy_old_main(argc, argv);
  /* Release the API use */
  mcapi_status_t status;
  mcapi_finalize(&status);
  SMECY_MCAPI_CHECK_STATUS(status);
}
#else
#define SMECY_start_PEs_dispatch                                        \
  /* Main function that starts all the MCAPI threads that runs on the
     accelerator */                                                     \
  int main() {                                                          \
    /* Create OpenMP threads to launch all MCAPI nodes instead of running
       the old main
    */                                                                  \
    _Pragma("omp parallel for num_threads(SMECY_CLUSTER_NB)")           \
    for(int smecy_cluster = 0; smecy_cluster < SMECY_CLUSTER_NB; ++smecy_cluster) { \
      _Pragma("omp parallel for num_threads(SMECY_PE_NB)")              \
      for(int smecy_pe = 0; smecy_pe < SMECY_PE_NB; ++smecy_pe)         \
        SMECY_accel_function_dispatch(smecy_cluster, smecy_pe);         \
    }                                                                   \
    return 0;                                                           \
  }
#endif
/* Implementation macros to deal with streaming */

/* Not implemented yet in MCAPI */
