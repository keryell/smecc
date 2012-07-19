/* SMECY low-level runtime implementation

   Ronan.Keryell@silkan.com
*/

#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#ifdef SMECY_VERBOSE
#include <stdio.h>
#define SMECY_PRINT_VERBOSE_RAW(...)                                    \
  fprintf(stderr, __VA_ARGS__)
/* With a ; to allow a statement or a declaration afterards */
#define SMECY_PRINT_VERBOSE(...) SMECY_PRINT_VERBOSE_RAW(__VA_ARGS__);
#define SMECY_PRINT_VERBOSE_COMMA(...)                                  \
  /* The , instead of ; is to have a single statement with the statement \
     following this macro. It allows for example to have this verbose   \
     macro between a #pragma omp section and the real statement. Do not \
     work before a declaration... */                                    \
  SMECY_PRINT_VERBOSE_RAW(__VA_ARGS__),
#else
#define SMECY_PRINT_VERBOSE(...)
#define SMECY_PRINT_VERBOSE_COMMA(...)
#endif


/* Load the implementations */
//#ifdef SMECY_OPENMP
#include <smecy_openmp.h>
//#endif


/* SMECY_IMP_ are the real implementations doing the real work, to be
   defined somewhere else. */

/* Do not use "do { ... } while(0)" trick to allow the macro to be used
   anywhere as an instruction since it create a new scope that prevent C99
   declarations of variables to be used later. */


/* Interface macros to deal with mapping and function executions */


#define SMECY_set(pe, instance, func)                                   \
  SMECY_PRINT_VERBOSE("Preparing to launch function \"%s\" on "         \
                      "processor \"%s\" n° %d\n", #func, #pe, instance) \
  SMECY_IMP_set(pe, instance, func)

#define SMECY_send_arg(pe, instance, func, arg, type, value)            \
  SMECY_PRINT_VERBOSE("Sending %s to function \"%s\" on processor "     \
                      "\"%s\" n° %d\n", #type, #func, #pe, instance)    \
  SMECY_IMP_send_arg(pe, instance, func, arg, type, value)

#define SMECY_cleanup_send_arg(pe, instance, func, arg, type, value)    \
  SMECY_PRINT_VERBOSE("Deal with post-sending %s to function"           \
                      " \"%s\" on processor "                           \
                      "\"%s\" n° %d\n", #type, #func, #pe, instance)    \
  SMECY_IMP_cleanup_send_arg(pe, instance, func, arg, type, value)

#define SMECY_send_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Sending vector of %zd elements of %s at address" \
                      " %p from argument %zd of function \"%s\" on "    \
                      "processor \"%s\" n° %d\n",                       \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_send_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_cleanup_send_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Deal with post-sending vector "                  \
                      "of %zd elements of %s at address"                \
                      " %p from argument %zd of function \"%s\" on "    \
                      "processor \"%s\" n° %d\n",                       \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_cleanup_send_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_update_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Update by sending vector "                       \
                      "of %zd elements of %s at address"                \
                      " %p from argument %zd of function \"%s\" on "    \
                      "processor \"%s\" n° %d\n",                       \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_update_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_cleanup_update_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Udate by receiving vector "              \
                      "of %zd elements of %s at address"                \
                      " %p from argument %zd of function \"%s\" on "    \
                      "processor \"%s\" n° %d\n",                       \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_cleanup_update_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_launch(pe, instance, func, n_args)                        \
  SMECY_PRINT_VERBOSE("Running function \"%s\" with %zd arguments on "  \
                      "processor \"%s\" n° %d\n",                       \
                      #func, (size_t) n_args, #pe, instance)            \
  SMECY_IMP_launch(pe, instance, func, n_args)

#define SMECY_prepare_get_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Preparing to receiving vector of %zd elements "  \
                      "of %s at address %p from argument %zd of "       \
                      "function \"%s\" on processor \"%s\" n° %d\n",    \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_prepare_get_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_get_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Receiving vector of %zd elements of %s at address" \
                      " %p from argument %zd of function \"%s\" on "    \
                      "processor \"%s\" n° %d\n",                       \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_get_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_get_return(pe, instance, func, type)                      \
  SMECY_PRINT_VERBOSE("Returning %s from function \"%s\" on processor"  \
                      " \"%s\" n° %d\n", #type, #func, #pe, instance)   \
  SMECY_IMP_get_return(pe, instance, func, type)



/* Interface macros to deal with streaming */

#define SMECY_stream_init(stream, nbstreams)                            \
  SMECY_PRINT_VERBOSE("Init stream %d with %d stages\n", stream, nbstreams) \
  SMECY_IMP_stream_init(stream, nbstreams)

#define SMECY_stream_launch(stream, stage)                               \
  /* Put the verbose information afterwards inside the implementation   \
     to cope with OpenMP constraints */                                 \
  SMECY_IMP_stream_launch(stream, stage)

#define SMECY_stream_get_init_buf(stream, stage)                         \
  SMECY_PRINT_VERBOSE("Init get buffer on stage %d from stream %d\n",    \
                      stage, stream)                                     \
  SMECY_IMP_stream_get_init_buf(stream, stage)

#define SMECY_stream_put_data(stream, stage)                            \
  SMECY_PRINT_VERBOSE("Post data for next stage on stage %d from stream %d\n", \
                      stage, stream)                                    \
  SMECY_IMP_stream_put_data(stream, stage);                             \
  SMECY_PRINT_VERBOSE("Sent the data for next stage on stage %d from stream %d\n", \
                      stage, stream)                                    \

#define SMECY_stream_get_data(stream, stage)                            \
  SMECY_PRINT_VERBOSE("Get data from previous stage on stage %d"        \
                      " from stream %d\n", stage, stream)               \
  SMECY_IMP_stream_get_data(stream, stage);                             \
  SMECY_PRINT_VERBOSE("Got the data from previous stage on stage %d"    \
                      " from stream %d\n", stage, stream)               \

#define SMECY_stream_copy_data(stream, stage)                            \
  SMECY_PRINT_VERBOSE("Copy data from previous stage to next stage unchanged" \
                      " on stage %d from stream %d\n", stage, stream)   \
  SMECY_IMP_stream_copy_data(stream, stage)

/* Wait for the end of the application with Unix system-call: */
#define SMECY_wait_for_the_end()                                        \
  /* Put the verbose information afterwards inside the implementation   \
     to cope with OpenMP constraints */                                 \
  SMECY_IMP_wait_for_the_end()

#endif //SMECY_LIB_H
