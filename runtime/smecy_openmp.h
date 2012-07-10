/* SMECY low-level runtime implementation with OpenMP

   Ronan.Keryell@silkan.com
*/

/* For the final pause() */
#include <unistd.h>


/* SMECY_IMP_ are the real implementations doing the real work, to be
   defined somewhere else. */

/* Implementation macros to deal with mapping and function executions */


// Create a variable name used to pass an argument to function
#define SMECY_IMP_VAR_ARG(pe, instance, func, arg)      \
  p4a_##pe##_##instance##_##func##_##arg

/* Wrapper that can be used for example to launch the function in another
   thread */
#define SMECY_IMP_LAUNCH_WRAPPER(func_call) func_call

// Implementations for the smecy library
#define SMECY_IMP_set(pe, instance, func)

#define SMECY_IMP_send_arg(pe, instance, func, arg, type, value)        \
  type SMECY_IMP_VAR_ARG(pe, instance, func, arg) = value

#define SMECY_IMP_cleanup_send_arg(pe, instance, func, arg, type, value)

#define SMECY_IMP_send_arg_vector(pe, instance, func, arg, type, addr, size) \
  type* SMECY_IMP_VAR_ARG(pe, instance, func, arg) = addr

#define  SMECY_IMP_cleanup_send_arg_vector(pe, instance, func, arg, type, addr, size)

#define  SMECY_IMP_update_arg_vector(pe, instance, func, arg, type, addr, size) \
  type* SMECY_IMP_VAR_ARG(pe, instance, func, arg) = addr

#define  SMECY_IMP_cleanup_update_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_IMP_launch(pe, instance, func, n_args)    \
  SMECY_IMP_launch_##n_args(pe, instance, func)

#define SMECY_IMP_prepare_get_arg_vector(pe, instance, func, arg, type, addr, size) \
  type* SMECY_IMP_VAR_ARG(pe, instance, func, arg) = addr

#define SMECY_IMP_get_arg_vector(pe, instance, func, arg, type, addr, size)

/* TODO: To be implemented... */
#define SMECY_IMP_get_return(pe, instance, func, type)

/* Implementation of the function calls themselves */
#define SMECY_IMP_launch_0(pe, instance, func)  \
  SMECY_IMP_LAUNCH_WRAPPER(func())


#define SMECY_IMP_launch_1(pe, instance, func)                          \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_VAR_ARG(pe, instance, func, 1)))

#define SMECY_IMP_launch_2(pe, instance, func)                          \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_VAR_ARG(pe, instance, func, 1), \
                                SMECY_IMP_VAR_ARG(pe, instance, func, 2)))



/* Implementation macros to deal with streaming */

// RK: je ne suis pas sûr que cette histoire de DbLink devrait apparaître
// Pourquoi int ?

#if 0
//prototypes for the stream library
typedef int DbLink;

DbLink pth_CreateDbLink(int size) { return (DbLink)0; }
void* DbLinkGetInitBuf(DbLink outputLink) { return NULL; }
void* DbLinkGetData(DbLink inputLink) { return NULL; }
void* DbLinkPutData(DbLink inputLink) { return NULL; }
int pth_CreateProcess(int (*f)(), ...) {return 0;}

#endif

/* Interface macros to deal with streaming */

#define SMECY_MAX_STREAMING_LOOPS 64
#define SMECY_MAX_STREAMING_NODES 32

static int smecy_next_stream = 0;
//DbLink smecy_streaming_buffers[SMECY_MAX_STREAMING_LOOPS][SMECY_MAX_STREAMING_NODES];

//TODO for loop and nbNodes param
#define SMECY_IMP_init_stream(stream, nbstreams)
#if 0
{\
        for (int smecy_iter_##stream=0; smecy_iter_##stream<(nbstreams); smecy_iter_##stream++)\
        {\
                smecy_streaming_buffers[smecy_next_stream][smecy_iter_##stream] = pth_CreateDbLink(sizeof(struct smecy_buffer_type_##stream)); \
        }\
        smecy_next_stream++; \
}
#endif

#define SMECY_IMP_launch_stream(stream, node)                           \
  //pth_CreateProcess(((int (*)())smecy_node_##stream##_##node))

#define SMECY_IMP_stream_get_init_buf(stream, node)\
  //smecy_struct_buffer_out = ((struct smecy_buffer_type_##stream *)(DbLinkGetInitBuf(smecy_streaming_buffers[stream][node])))

#define SMECY_IMP_stream_put_data(stream, node)                         \
  //smecy_struct_buffer_out = ((struct smecy_buffer_type_##stream *)(DbLinkPutData(smecy_streaming_buffers[stream][node])))

#define SMECY_IMP_stream_get_data(stream, node)                         \
  //smecy_struct_buffer_in = ((struct smecy_buffer_type_##stream *)(DbLinkPutData(smecy_streaming_buffers[stream][node])))

#define SMECY_IMP_stream_copy_data(stream, node)                \
  //smecy_struct_buffer_out = smecy_struct_buffer_in

/* Wait to the end of the application with Unix system-call: */
#define SMECY_IMP_wait_for_the_end() pause()
