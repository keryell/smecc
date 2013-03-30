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
/* Needed to display verbose MCAPI error messages: */
#include <stdio.h>
#include <stdarg.h>

#ifdef SMECY_MCAPI_HOST
/* For SMECY_MCAPI_connection */
#include <stdbool.h>
#endif

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


#ifdef MCAPI_STHORM
/* Use the API to deal with threads on the fabric: */
#include "mcapi_fabric_helper.h"
/* For some timing functions */
//#include "mcapi_time_helper.h"
#endif

#if defined(MCAPI_STHORM) && defined(SMECY_MCAPI_CHECK_TRACE)
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


/* MCAPI initialization parameters */
#ifdef MCAPI_STHORM
/* This is how we specify the library to launch on the STHORM accelerator
   fabric */
static mcapi_param_t SMECY_mcapi_param_init = "libsmecy_accel_fabric.so";
#define SMECY_MCAPI_PARAM_INIT (&SMECY_mcapi_param_init)
#else
#define SMECY_MCAPI_PARAM_INIT NULL
#endif


/* A macro to get debug information on a channel handle such as
   receive_gate */
#ifdef MCAPI_STHORM
/* Peek into the implementation-specific mcapi_chan_hndl_t to get a
   field: */
#define SMECY_CHAN_INFO(handle) ((intptr_t)handle.queueDesc)
#else
/* Else, guess it is a pointer or int-like type: */
#define SMECY_CHAN_INFO(handle) ((intptr_t)handle)
#endif


/* Compute the port used used on the host side to communicate for RX or TX
   on MCAPI domain node*/
#define SMECY_MCAPI_PORT(TX_or_RX,domain,node) \
  (SMECY_MCAPI_HOST_##TX_or_RX##_STARTING_PORT + SMECY_PE_NB*domain + node)

/* Machine description */
enum {
  /* The STHORM geometry */
  SMECY_CLUSTER_NB = 4,
  SMECY_PE_NB = 16,
  /* The localization of the host inside the MCAPI realm: */
  SMECY_MCAPI_HOST_DOMAIN = 5,
  SMECY_MCAPI_HOST_NODE = 0,
  /* The port numbers to connect a PE to a host. Since a PE is only
     connected to the host, only to endpoints and thus ports are needed. */
  SMECY_MCAPI_PE_TX_PORT = 1,
  SMECY_MCAPI_PE_RX_PORT = 2,
  /* Since MCAPI does not allow multiple connections on a same port, use
     on the host a different port to connect to each PE. */
  /* There are not a lot of ports on STHORM, so use the lower number to
     start: */
  SMECY_MCAPI_HOST_TX_STARTING_PORT = 0x0,
  /* Allocate the reception ports just after the transmission ones: */
  SMECY_MCAPI_HOST_RX_STARTING_PORT =
    SMECY_MCAPI_PORT(TX,SMECY_CLUSTER_NB,0),
};


/* To get a printf-like feedback.

   Since MCAPI_TRACE_C add a new line, add a "\n" to non MCAPI_STHORM case
   too
*/
static SMECY_printf_varargs(const char *format, va_list ap) {
#ifdef MCAPI_STHORM
  /* Well, cannot use asprintf on MCAPI... */
  static char big_message[1000];
  vsnprintf(big_message, sizeof(big_message), format, ap);
  MCAPI_TRACE_C(big_message);
#else
  vfprintf(stderr, format, ap);
  /* Note that the new line is not atomic because it is in another
     printf... */
  fprintf(stderr, "\n");
#endif
}

static SMECY_printf(const char *format,  ...) {
  va_list ap;
  va_start(ap, format);
  SMECY_printf_varargs(format, ap);
  va_end(ap);
}

#ifdef SMECY_VERBOSE
#include <stdio.h>
/* Prefix all the debug messages with "SMECY: " to ease filtering */
#define SMECY_PRINT_VERBOSE_RAW(...)                                    \
  SMECY_printf("SMECY: " __VA_ARGS__)
/* With a ; to allow a statement or a declaration afterards

   The comment on 2 lines at the end is to force a poor-man formating of
   the output when using cpp -CC (preprocess but keep the comments) to
   separate better the debug message from the real code
*/
#define SMECY_PRINT_VERBOSE(...) SMECY_PRINT_VERBOSE_RAW(__VA_ARGS__);/*
*/
#define SMECY_PRINT_VERBOSE_COMMA(...)                                  \
  /* The , instead of ; is to have a single statement with the statement \
     following this macro. It allows for example to have this verbose   \
     macro between a #pragma omp section and the real statement. Do not \
     work before a declaration... */                                    \
  SMECY_PRINT_VERBOSE_RAW(__VA_ARGS__),/*
*/
#else
#define SMECY_PRINT_VERBOSE(...)
#define SMECY_PRINT_VERBOSE_COMMA(...)
#endif

/* Test error code.

   Display and exit on failure
*/
void static SMECY_MCAPI_check_status(mcapi_status_t status,
                                     char file[],
                                     const char function[],
                                     int line,
                                     const char *format,
                                     ...) {
#ifndef SMECY_MCAPI_CHECK_TRACE
  if (status != MCAPI_SUCCESS) {
    /* Something went wrong */
#endif
  /* If SMECY_MCAPI_CHECK_TRACE is set for asking trace mode, display
     verbose status message even without any error */
  if (format[0] != '\0') {
    /* We have a message to display */
    SMECY_printf(" - From file '%s', function '%s', line %d:",
                 file, function, line);
    va_list ap;
    va_start(ap, format);
    SMECY_printf_varargs(format, ap);
    va_end(ap);
  }
#ifdef SMECY_MCAPI_CHECK_TRACE
  if (status != MCAPI_SUCCESS) {
    /* Something went wrong */
#endif
#ifdef SMECY_VERBOSE
    /* Use a function from Linux MCAPI implementation to get a string
       translation of the status: */
#ifndef MCAPI_MAX_STATUS_SIZE
#define MCAPI_MAX_STATUS_SIZE 250
#endif
#ifdef MCAPI_STHORM
    //#define puts(...)
    char *message = "";
#else
    char message[MCAPI_MAX_STATUS_SIZE];
    mcapi_display_status(status, message, MCAPI_MAX_STATUS_SIZE);
#endif
    SMECY_printf("API call fails in file '%s', function '%s',"
                 " line %d with error:\n\t%s",
                 file, function, line, message);
#ifdef MCAPI_STHORM
    /* Rely on the STHORM MCAPI tracing API to display the status */
    MCAPI_TRACE_C(format);
    MCAPI_TRACE_CS(function, status);
    MCAPI_TRACE_CI(file, line);
#endif
#endif
#ifndef MCAPI_STHORM
    /* Exit and forward the error code to the OS: */
    exit(status);
#else
    /* Well, there is no exit() on STHORM, so loop around instead of going
       on with nasty side effects... */
    MCAPI_TRACE_C("This thread is waiting for ever because of an error.\n");
    for(;;) ;
#endif
  }
  /* Go on, no error */
  return;
}


/* The wrapping macro for MCAPI_check_status to capture the call site
   information */
#define SMECY_MCAPI_CHECK_STATUS(status)                                \
  SMECY_MCAPI_check_status(status, __FILE__, __func__, __LINE__, "")

#define SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, ...)                   \
  SMECY_MCAPI_check_status(status, __FILE__, __func__, __LINE__, __VA_ARGS__)



/* SMECY_IMP_ are the real implementations doing the real work, to be
   defined somewhere else. */

/* Implementation macros to deal with mapping and function executions */


// Create a unique variable name used to pass an argument to function
#define SMECY_IMP_VAR_ARG(func, arg, pe, ...)	\
  p4a_##pe##_##func##_##arg

// Create a unique variable name for a message
#define SMECY_IMP_VAR_MSG(func, arg, pe, ...)	\
  p4a_##pe##_##func##_##arg##_msg

//  SMECY_CONCAT(SMECY_CONCAT(p4a_##pe##_,SMECY_CONCATENATE(__VA_ARGS__)),##_##func##_##arg)

/* Wrapper that can be used for example to launch the function in another
   thread */
#define SMECY_IMP_LAUNCH_WRAPPER(func_call) func_call


// Implementations for the SMECY library on MCAPI

/*
  Communications are based on MCAPI packet channels, the MCAPI
  connected-mode connections.

  The host open connections on-demand to the accelerator PEs and cache the
  connections to avoid spending time every-time.

  A PE tries to open a connection with the host and is blocked until the
  host need it. The the PE enter an infinite dispatching loop so there is
  no caching needed here (hopefuly, since the accelerators are not that
  memory proficient...).
 */


/* Start a connection request */
static void
SMECY_MCAPI_connect(mcapi_endpoint_t pkt_send, mcapi_endpoint_t pkt_receive) {
  mcapi_request_t handle;
  mcapi_status_t status;
  size_t size;
  /* Connect both ends */
  mcapi_pktchan_connect_i(pkt_send, pkt_receive, &handle, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_pktchan_connect_i "
                                   "on send endpoint %#tx from receive "
                                   "endpoint %#tx with handle %#tx",
                                   (intptr_t)pkt_send, (intptr_t)pkt_receive,
                                   (intptr_t)handle);
  /* ...and wait for its completion */
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_wait on handle %#tx "
                                   "returned size %zx", (intptr_t)handle,
                                   size);
}


/* Create a packet channel for reception listening on receive_port */
mcapi_pktchan_recv_hndl_t static
SMECY_MCAPI_receive_gate_create(mcapi_port_t receive_port,
                                mcapi_domain_t send_domain,
                                mcapi_node_t send_node,
                                mcapi_port_t send_port) {
  mcapi_status_t status;
  /* Create the local endpoint for reception */
  mcapi_endpoint_t pkt_receive = mcapi_endpoint_create(receive_port, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_endpoint_create"
                                   " with receive_port %#tx returns"
                                   " pkt_receive %#tx",
                                   (intptr_t)receive_port,
                                   (intptr_t)pkt_receive);

#if defined(MCAPI_STHORM) && !defined(SMECY_MCAPI_HOST)
  /* On STHORM fabric side, it looks like we need to precise this
     attribute to have a connection with the host working */
  mcapi_endp_attr_memory_type_t memtype = MCAPI_ENDP_ATTR_REMOTE_MEMORY;
  mcapi_endpoint_set_attribute(pkt_receive,
                               MCAPI_ENDP_ATTR_MEMORY_TYPE,
                               &memtype,
                               sizeof(mcapi_endp_attr_memory_type_t),
                               &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_endpoint_set_attribute "
                                   "MCAPI_ENDP_ATTR_REMOTE_MEMORY");
#endif

  mcapi_request_t handle;

#ifdef SMECY_MCAPI_HOST
  /* Choose to do the connection request on the host size to spare
     resources on the accelerator side */

  /* Get the remote end point. Wait if it is not created at the receive
     side */
  mcapi_endpoint_t pkt_send = mcapi_endpoint_get(send_domain,
                                                 send_node,
                                                 send_port,
                                                 MCA_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_endpoint_get "
                                   "on send domain %#tx, node %#tx and "
                                   "port %#tx returns pkt_send %#tx",
                                   (intptr_t)send_domain, (intptr_t)send_node,
                                   (intptr_t)send_port, (intptr_t)pkt_send);

  SMECY_MCAPI_connect(pkt_send, pkt_receive);
#endif

  mcapi_pktchan_recv_hndl_t receive_gate;
  /* Start a connection request... */
  mcapi_pktchan_recv_open_i(&receive_gate, pkt_receive, &handle, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_pktchan_recv_open_i "
                                   "on receive port %#tx on gate %#tx and "
                                   "handle %#tx", (intptr_t)pkt_receive,
                                   SMECY_CHAN_INFO(receive_gate),
                                   (intptr_t)handle);
  /* ...and wait for its completion */
  size_t size;
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_wait on handle %#tx "
                                   "returned size %zx", (intptr_t)handle, size);
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
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_endpoint_create "
                                   "on send port %#tx returns pkt_send %#tx",
                                   (intptr_t)send_port, (intptr_t)pkt_send);

#if defined(MCAPI_STHORM) && !defined(SMECY_MCAPI_HOST)
  /* On STHORM fabric side, it looks like we need to precise this
     attribute to have a connection with the host working */
  mcapi_endp_attr_memory_type_t memtype = MCAPI_ENDP_ATTR_REMOTE_MEMORY;
  mcapi_endpoint_set_attribute(pkt_send,
                               MCAPI_ENDP_ATTR_MEMORY_TYPE,
                               &memtype,
                               sizeof(mcapi_endp_attr_memory_type_t),
                               &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_endpoint_set_attribute "
                                   "MCAPI_ENDP_ATTR_REMOTE_MEMORY");
#endif

  mcapi_request_t handle;
  size_t size;

#ifdef SMECY_MCAPI_HOST
  /* Choose to do the connection request on the host size to spare
     resources on the accelerator side */

  /* Get the remote end point. Wait if it is not created at the receive
     side */
  mcapi_endpoint_t pkt_receive = mcapi_endpoint_get(receive_domain,
						    receive_node,
						    receive_port,
						    MCA_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_endpoint_get "
                                   "on receive domain %#tx, node %#tx and "
                                   "port %#tx returns pkt_receive %#tx",
                                   (intptr_t)receive_domain, (intptr_t)receive_node,
                                   (intptr_t)receive_port, (intptr_t)pkt_receive);

  SMECY_MCAPI_connect(pkt_send, pkt_receive);
#endif

  mcapi_pktchan_send_hndl_t send_gate;
  /* Open this side of the channel for sending... */
  mcapi_pktchan_send_open_i(&send_gate, pkt_send, &handle, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_pktchan_send_open_i "
                                   "send gate %#tx, send endpoint %#tx "
                                   "with handle %#tx",
                                   SMECY_CHAN_INFO(send_gate),
                                   (intptr_t)pkt_send,
                                   (intptr_t)handle);
  /* And wait for the opening */
  mcapi_wait(&handle, &size, MCAPI_TIMEOUT_INFINITE, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "mcapi_wait on handle %#tx "
                                   "returned size %zx", (intptr_t)handle,
                                   size);
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

/* Map a 1-dimension PE number on MCAPI */
#define SMECY_MCAPI_PARSE_PE_PE(p)              \
/* Just select domain 0 */                      \
  SMECY_MCAPI_PARSE_PE_STHORM(0, p)

#if 0
/* Do not allow the map(Host...) since it leads to deadlocks (we are
   already on the host!) */

/* Analyze the Host coordinates, which are not specified in the pragma.
   Replace them with the host MCAPI node */
#define SMECY_MCAPI_PARSE_PE_Host()                     \
  mcapi_domain_t domain = SMECY_MCAPI_HOST_DOMAIN;/*
                                                   */   \
  mcapi_node_t node = SMECY_MCAPI_HOST_NODE
#endif

#ifdef SMECY_MCAPI_HOST
/* Scoreboard used to keep the status of a connection between the host
   and each PE.

   Since it is a global array, it is default-initialized to 0, which is good.

   TODO: use a bitmask instead of a char array to save memory. But more
   complex to insure atomicity at the bit level...
*/
struct {
    bool opened;
    mcapi_pktchan_send_hndl_t transmit;
    mcapi_pktchan_recv_hndl_t receive;
  } SMECY_MCAPI_connection[SMECY_CLUSTER_NB][SMECY_PE_NB];
#endif

static void SMECY_IMP_finalize() {
  mcapi_status_t status;
  /* Release the API use */
  mcapi_finalize(&status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "Finalizing MCAPI");
}


static void SMECY_IMP_initialize_then_finalize() {
  //mcapi_node_attributes_t node_attributes;
  mcapi_info_t info;
  mcapi_status_t status;

#ifdef SMECY_MCA_API_DEBUG_LEVEL
  /* Set the requested debug level of the MCA API itself */
  mcapi_set_debug_level(SMECY_MCA_API_DEBUG_LEVEL);
#endif
  // init node attributes. Not clear in which MCAPI version it is needed...
  /* It looks like in the Linux MCAPI implementation reference from MCA,
     even the 2.015 version looks like a V1 interface... */
#if (MCAPI_VERSION >= 2000)
  mcapi_node_attributes_t node_attributes;
  mcapi_node_init_attributes(&node_attributes, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "Initializing MCAPI attributes");
  /* 6 arguments in V.2 */
  mcapi_initialize(SMECY_MCAPI_HOST_DOMAIN, SMECY_MCAPI_HOST_NODE,
  		   &node_attributes, SMECY_MCAPI_PARAM_INIT, &info, &status);
  MCAPI_TRACE_C("Host initialization V.2 done");
#else
  /* 5 arguments in V.1 */
  mcapi_initialize(SMECY_MCAPI_HOST_DOMAIN, SMECY_MCAPI_HOST_NODE,
		   SMECY_MCAPI_PARAM_INIT, &info, &status);
  MCAPI_TRACE_C("Host initialization V.1 done");
#endif
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "Initializing MCAPI on domain"
                                   " %#tx and node %#tx",
                                   (intptr_t) SMECY_MCAPI_HOST_DOMAIN,
                                   (intptr_t) SMECY_MCAPI_HOST_NODE);
  /* And the register the finalization for an execution at the end of the
     program execution */
  atexit(SMECY_IMP_finalize);
}

#ifdef SMECY_MCAPI_HOST
/* Open some MCAPI connections with the requested node

   Only create a connection once by using a scoreboard to keep connection
   status to each PE.

   TODO: split this macro with sub-functions
 */
#define SMECY_IMP_set(func, instance, pe, ...)                          \
  SMECY_LBRACE /* To have local variables
                */                                                      \
  mcapi_status_t SMECY_MCAPI_status; /*
  Analyze the PE type and coordinates into domain & node */             \
  SMECY_MCAPI_PARSE_PE(pe, __VA_ARGS__);                                \
  /* The handle to sending packets
   */                                                                   \
  mcapi_pktchan_send_hndl_t P4A_transmit;                               \
  /* The handle to receiving packets
   */                                                                   \
  mcapi_pktchan_recv_hndl_t P4A_receive;                                \
  /* To be thread safe on the caching system.

     That means a lot of code in this critical section, in case there are
     different SMECY_IMP_set at the same time.  A bit overkill, to be
     streamlined some day...

     It is specially VERY slow in verbose mode since the message are
     displayed inside the critical section.
   */                                                                   \
  _Pragma("omp critical(SMECY_IMP_set)")                                \
  {                                                                     \
    /* No need for an OpenMP flush because of the critical section */   \
    if (SMECY_MCAPI_connection[domain][node].opened) {                  \
      P4A_transmit = SMECY_MCAPI_connection[domain][node].transmit;     \
      P4A_receive = SMECY_MCAPI_connection[domain][node].receive;       \
      SMECY_PRINT_VERBOSE("SMECY_IMP_set: cache hit for domain %d, "    \
                          "node %d: P4A_transmit = %#tx,"               \
                          " P4A_receive = %#tx",                        \
                          domain, node, SMECY_CHAN_INFO(P4A_transmit),  \
                          SMECY_CHAN_INFO(P4A_receive));                \
    }                                                                   \
    else {                                                              \
      /* This is not already opened, create the connections.

                                Do it in this order compared with the PE
                                to avoid dead-locks on opening: first open
                                a connection to send data to the PE */  \
      P4A_transmit = SMECY_MCAPI_send_gate_create(SMECY_MCAPI_PORT(TX,domain,node), \
                                                  domain,               \
                                                  node,                 \
                                                  SMECY_MCAPI_PE_RX_PORT); /*
                  Then open a connection to receive data from the PE */ \
      P4A_receive = SMECY_MCAPI_receive_gate_create(SMECY_MCAPI_PORT(RX,domain,node), \
                                                    domain,             \
                                                    node,               \
                                                    SMECY_MCAPI_PE_TX_PORT);/*
                                                                         */ \
      /* No need for an OpenMP flush because of the critical section */ \
      SMECY_MCAPI_connection[domain][node].transmit = P4A_transmit;     \
      SMECY_MCAPI_connection[domain][node].receive = P4A_receive;       \
      SMECY_MCAPI_connection[domain][node].opened = true;               \
    }                                                                   \
  }                                                                     \
  /* Send the function name to run to the remode dispatcher,
     including the final '\0' */                                        \
  size_t length = strlen(#func) + 1;                                    \
  mcapi_pktchan_send(P4A_transmit,                                      \
                     #func,                                             \
                     length,                                            \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status,                  \
                                   "mcapi_pktchan_send to send gate"    \
                                   " %#tx '%s' of length %#tx",         \
                                   SMECY_CHAN_INFO(P4A_transmit),       \
                                   #func, length);                      \
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
  mcapi_pktchan_send(P4A_transmit,                                      \
                     &SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__),    \
                     sizeof(type),                                      \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status, "mcapi_pktchan_send " \
                                   "to send gate %#tx %p of length %zx", \
                                   SMECY_CHAN_INFO(P4A_transmit),       \
                                   &SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__), sizeof(type));
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
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status, "mcapi_pktchan_recv " \
                                   "from receive gate %#tx %p of length %zx", \
                                   P4A_receive,                         \
                                   (void **)&SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__), \
                                   P4A_received_size);                  \
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
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status,                  \
                                   "mcapi_pktchan_release %p",          \
                                   SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__))
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  /* Send the vector data to the PE
   */                                                                   \
  mcapi_pktchan_send(P4A_transmit, addr, size, &SMECY_MCAPI_status);    \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status, "mcapi_pktchan_send " \
                                   "to send gate %#tx %p of length %zx", \
                                   SMECY_CHAN_INFO(P4A_transmit), addr, size);
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
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status, "mcapi_pktchan_recv " \
                                   "from receive gate %#tx %p of length %zx", \
                                   SMECY_CHAN_INFO(P4A_receive),        \
                                   (void **)&SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__), \
                                   P4A_received_size);                  \
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
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status,                  \
                                   "mcapi_pktchan_release %p",          \
                                   SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__))
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
  SMECY_PRINT_VERBOSE("Preparing to receiving vector of %zd elements "  \
                      "of %s at address %p from arg #%d of "            \
                      "function \"%s\" on processor \"%s\" n° \"%s\"",  \
                      (size_t) size, #type, addr, arg,                  \
                      #func, #pe, #__VA_ARGS__)                         \
  /* The pointer to the packet received from the PE by MCAPI
   */                                                           \
  type* SMECY_IMP_VAR_MSG(func, arg, pe, __VA_ARGS__)
#else
/* This is on the accelerator side */
#define SMECY_IMP_prepare_get_arg_vector(func, arg, type, addr, size, pe, ...) \
  /* Allocate the memory given to the function to receive the data
     from the execution */                                               \
  type SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__)[size];              \
  /* Display the address only now since it is defined by the previous
     allocation.  */                                                     \
  SMECY_PRINT_VERBOSE("Preparing to receiving vector of %zd elements "   \
                      "of %s at address %p from arg #%d of "             \
                      "function \"%s\" on processor \"%s\" n° \"%s\"",   \
                      (size_t) size, #type,                              \
                      SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__),     \
                      arg, #func, #pe, #__VA_ARGS__)
#endif


#ifdef SMECY_MCAPI_HOST
#define SMECY_IMP_get_arg_vector(func, arg, type, addr, size, pe, ...)  \
  SMECY_PRINT_VERBOSE("Receiving vector of %zd elements of %s at address" \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"", (size_t) size,      \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)       \
  /* Receive the vector result from the accelerator
   */                                                                   \
  mcapi_pktchan_recv(P4A_receive,                                       \
                     (void **)&SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__), \
                     &P4A_received_size,                                \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status, "mcapi_pktchan_recv " \
                                   "from receive gate %#tx %p of length %zx", \
                                   SMECY_CHAN_INFO(P4A_receive),        \
                                   (void **)&SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__), \
                                   P4A_received_size);                  \
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
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status,                  \
                                   "mcapi_pktchan_release %p",          \
                                   SMECY_IMP_VAR_MSG(func,arg,pe,__VA_ARGS__))
#else
/* This is on the accelerator side */
#define SMECY_IMP_get_arg_vector(func, arg, type, addr, size, pe, ...)  \
  SMECY_PRINT_VERBOSE("Receiving vector of %zd elements of %s at address" \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"", (size_t) size,      \
                      #type, SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__), \
                      arg, #func, #pe, #__VA_ARGS__)                    \
  /* Send the vector data given by the function execution back to the host
   */                                                                   \
  mcapi_pktchan_send(P4A_transmit,                                      \
                     SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__),     \
                     size*sizeof(type),                                 \
                     &SMECY_MCAPI_status);                              \
  /* Check the correct execution
   */                                                                   \
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status, "mcapi_pktchan_send " \
                                   "to send gate %#tx %p of length %zx", \
                                   SMECY_CHAN_INFO(P4A_transmit),       \
                                   SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__), \
                                   size*sizeof(type));
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
static void SMECY_init_mcapi_node(int smecy_cluster, int smecy_pe) {
  mcapi_info_t info;
  mcapi_status_t status;

#ifdef SMECY_MCA_API_DEBUG_LEVEL
  /* Set the requested debug level of the MCA API itself */
  mcapi_set_debug_level(SMECY_MCA_API_DEBUG_LEVEL);
#endif
  // init node attributes. Not clear in which MCAPI version it is needed...
  /* It looks like in the Linux MCAPI implementation reference from MCA,
     even the 2.015 version looks like a V1 interface... */
#if (MCAPI_VERSION >= 2000)
  mcapi_node_attributes_t node_attributes;
  mcapi_node_init_attributes(&node_attributes, &status);
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "Initializing MCAPI attributes");
  /* 6 arguments in V.2 */
  mcapi_initialize(smecy_cluster, smecy_pe,
  		   &node_attributes, NULL, &info, &status);
  MCAPI_TRACE_C("Fabric initialization V.2 done");
#else
  /* 5 arguments in V.1 */
  mcapi_initialize(smecy_cluster, smecy_pe,
		   NULL, &info, &status);
  MCAPI_TRACE_C("Fabric initialization V.1 done");
#endif
  SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "Initialization of smecy_cluster"
                                   " %d, smecy_pe %d",
                                   smecy_cluster, smecy_pe);
}

#define SMECY_begin_accel_function_dispatch                             \
  /* The dispatch function to be run on a PE
   */                                                                   \
  void SMECY_accel_function_dispatch(int smecy_cluster, int smecy_pe) SMECY_LBRACE \
    /*
     Initialize MCAPI
     */                                                                   \
    SMECY_init_mcapi_node(smecy_cluster, smecy_pe);                       \
    /* Create the channels to communicate with the host
     */                                                                   \
    mcapi_pktchan_send_hndl_t P4A_receive =                               \
      SMECY_MCAPI_receive_gate_create(SMECY_MCAPI_PE_RX_PORT,             \
                                      SMECY_MCAPI_HOST_DOMAIN,            \
                                      SMECY_MCAPI_HOST_NODE,              \
                                      SMECY_MCAPI_PORT(TX,smecy_cluster,smecy_pe)); /*
                                                                */        \
    mcapi_pktchan_send_hndl_t P4A_transmit =                              \
      SMECY_MCAPI_send_gate_create(SMECY_MCAPI_PE_TX_PORT,                \
                                   SMECY_MCAPI_HOST_DOMAIN,               \
                                   SMECY_MCAPI_HOST_NODE,                 \
                                   SMECY_MCAPI_PORT(RX,smecy_cluster,smecy_pe)); /*
      Enter the infinite service loop on the PE
    */                                                                    \
    for(;;) SMECY_LBRACE                                                  \
      SMECY_PRINT_VERBOSE("PE %d %d is waiting for a job",                \
                           smecy_cluster, smecy_pe)                       \
      /* Wait for the function name to run:
       */                                                                 \
      char *function_name;                                                \
      size_t P4A_received_size;                                           \
      mcapi_status_t SMECY_MCAPI_status;                                  \
      mcapi_pktchan_recv(P4A_receive,                                     \
                         (void **)&function_name,                         \
                         &P4A_received_size,                              \
                         &SMECY_MCAPI_status);                            \
      /* Check the correct execution
       */                                                                 \
      SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status,                \
                                       "mcapi_pktchan_recv from receive " \
                                       "gate %#tx '%s' of length %zx",    \
                                       SMECY_CHAN_INFO(P4A_receive),      \
                                       function_name,                     \
                                       P4A_received_size);                \
      do SMECY_LBRACE                                                     \
        /* Find the right function to run
         */


#define SMECY_end_accel_function_dispatch                               \
        /* If we get here, we did not encounter a break and so nothing  \
           matches the requested function */                            \
        SMECY_printf("No candidate function to execute \"%s\" on PE %d %d", \
                     function_name, smecy_cluster, smecy_pe);           \
        exit(-1);                                                       \
      SMECY_RBRACE while (0);                                           \
      mcapi_pktchan_release(function_name,                              \
                            &SMECY_MCAPI_status);                       \
      /* Check the correct execution                                    \
       */                                                               \
      SMECY_MCAPI_CHECK_STATUS_MESSAGE(SMECY_MCAPI_status,              \
                                       "mcapi_pktchan_release %p",      \
                                       function_name);                  \
      SMECY_MCAPI_CHECK_STATUS(SMECY_MCAPI_status);                     \
      /* End of the PE accelerator polling loop.                        \
       */                                                               \
    SMECY_RBRACE                                                        \
  /* There is no finalize because of the infinite loop in               \
     smecy_accel_function_dispatch                                      \
  */                                                                    \
  SMECY_RBRACE


/* The parameter given to the accelerator function */
#define SMECY_accel_func_args                           \
           mcapi_pktchan_send_hndl_t P4A_transmit,      \
           mcapi_pktchan_recv_hndl_t P4A_receive

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
                        " of function \"" #function "\"",               \
                        smecy_cluster, smecy_pe)                        \
    smecy_accel_##function##_##instance(P4A_transmit, P4A_receive);     \
    /* Wait for next job to do
     */                                                                 \
    break;                                                              \
  }


#ifdef SMECY_MCAPI_HOST
/* Nothing to do here since smecc should have injected at the begining of
   the main() function a call to SMECY_initialize_then_finalize that
   initialize MCAPI on the host at the begining and finilize MCAPI at the
   end of the program. */
#else
#ifdef MCAPI_STHORM

#define SMECY_start_PEs_dispatch                                        \
  /* Get the MCAPI coordinates before registering to MCAPI */           \
  void SMECY_thread_entry(void *args) {                                 \
    MCAPI_TRACE_C("Local PE thread started");                           \
    mcapi_status_t status;                                              \
    mcapi_domain_t domain_id = mcapi_domain_id_get(&status);            \
    SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "Get the domain id");      \
    mcapi_node_t node_id = mcapi_node_id_get(&status);                  \
    SMECY_MCAPI_CHECK_STATUS_MESSAGE(status, "Get the node id");        \
    SMECY_accel_function_dispatch(domain_id, node_id);                  \
  }                                                                     \
                                                                        \
  /* This is called by the STHORM MCAPI run-time on all the fabric
     clusters (= MCAPI domains) */                                      \
  void mcapi_domain_entry() {                                           \
    MCAPI_TRACE_C("Local cluster started");                             \
    /* Create all the threads on the nodes of the current cluster */    \
    for(int node_id = 0; node_id != SMECY_PE_NB; ++node_id)             \
      create_node(node_id, SMECY_thread_entry, NULL, NULL, NULL);       \
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
#endif


/* Implementation macros to deal with streaming */

/* Not implemented yet in MCAPI */
