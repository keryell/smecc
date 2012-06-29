/* SMECY low-level runtime implementation

   Ronan.Keryell@silkan.com
*/

#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#ifdef SMECY_VERBOSE
#include <stdio.h>
#define SMECY_PRINT_VERBOSE(...) \
  fprintf(stderr, __VA_ARGS__);
#else
#define SMECY_PRINT_VERBOSE(...)
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

#define SMECY_cleanup_send_arg(pe, instance, func, arg, type, value)	\
  SMECY_PRINT_VERBOSE("Deal with post-sending %s to function"		\
		      " \"%s\" on processor "				\
                      "\"%s\" n° %d\n", #type, #func, #pe, instance)    \
  SMECY_IMP_cleanup_send_arg(pe, instance, func, arg, type, value)

#define SMECY_send_arg_vector(pe, instance, func, arg, type, value, size) \
  SMECY_PRINT_VERBOSE("Sending vector of %zd elements of %s to "        \
                      "function  \"%s\" on processor \"%s\" n° %d\n",   \
                      (size_t) #size, #type, #func, #pe, instance)      \
  SMECY_IMP_send_arg_vector(pe, instance, func, arg, type, value, size)

#define SMECY_cleanup_send_arg_vector(pe, instance, func, arg, type, value, size) \
  SMECY_PRINT_VERBOSE("Deal with post-sending vector of"		\
		      " %zd elements of %s to "				\
                      "function  \"%s\" on processor \"%s\" n° %d\n",   \
                      (size_t) #size, #type, #func, #pe, instance)      \
  SMECY_IMP_cleanup_send_arg_vector(pe, instance, func, arg, type, value, size)

#define SMECY_update_arg_vector(pe, instance, func, arg, type, value, size) \
  SMECY_PRINT_VERBOSE("Update by sending vector of %zd elements of %s to " \
                      "function  \"%s\" on processor \"%s\" n° %d\n",   \
                      (size_t) #size, #type, #func, #pe, instance)      \
  SMECY_IMP_update_arg_vector(pe, instance, func, arg, type, value, size)

#define SMECY_cleanup_update_arg_vector(pe, instance, func, arg, type, value, size) \
  SMECY_PRINT_VERBOSE("Udate by receiving-sending vector of"		\
		      " %zd elements of %s to "				\
                      "function  \"%s\" on processor \"%s\" n° %d\n",   \
                      (size_t) #size, #type, #func, #pe, instance)      \
  SMECY_IMP_cleanup_update_arg_vector(pe, instance, func, arg, type, value, size)

// Old stuff to clean...
#if 0
// Note that it can only work if the code maps only the square_symmetry
// function
#define SMECY_launch(pe, instance, func, n_args)                        \
  { if (#func == "square_symmetry")                                     \
      square_symmetry_smecy( pe##_square_symmetry_1,                    \
                             pe##_square_symmetry_2,                    \
                             (int*)pe##_square_symmetry_3,              \
                             pe##_square_symmetry_4,                    \
                             pe##_square_symmetry_5,                    \
                             pe##_square_symmetry_6);                   \
  }
#else
#define SMECY_launch(pe, instance, func, n_args)                        \
  SMECY_PRINT_VERBOSE("Running function \"%s\" with %zd arguments on "  \
                      "processor \"%s\" n° %d\n",                       \
                      #func, (size_t) n_args, #pe, instance)            \
  SMECY_IMP_launch(pe, instance, func, n_args)
#endif

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


//void square_symmetry_smecy(int width, int height, int* image,
//                     int square_size, int x_offset, int y_offset) ;


/* Interface macros to deal with streaming */

#define SMECY_init_stream(stream, nbstreams)                            \
  SMECY_PRINT_VERBOSE("Init stream %d among 0..%d\n", stream, nbstreams) \
  SMECY_IMP_init_stream(stream, nbstreams)

#define SMECY_launch_stream(stream, node)                               \
  SMECY_PRINT_VERBOSE("Launch node %d from stream %d\n", node, stream)  \
  SMECY_IMP_launch_stream(stream, node)

#define SMECY_stream_get_init_buf(stream, node)                         \
  SMECY_PRINT_VERBOSE("Init get buffer on node %d from stream %d\n",    \
                      node, stream)                                     \
  SMECY_IMP_stream_get_init_buf(stream, node)

#define SMECY_stream_put_data(stream, node)                             \
  SMECY_PRINT_VERBOSE("Post data for next stage on node %d from stream %d\n", \
                      node, stream)                                     \
  SMECY_IMP_stream_put_data(stream, node)

#define SMECY_stream_get_data(stream, node)                     \
  SMECY_PRINT_VERBOSE("Get data from previous stage on node %d" \
                      " from stream %d\n", node, stream)        \
  SMECY_IMP_stream_get_data(stream, node)

#define SMECY_stream_copy_data(stream, node)                            \
  SMECY_PRINT_VERBOSE("Copy data from previous stage to next stage"     \
                      " unchanged on node %d from stream %d\n", node, stream) \
  SMECY_IMP_stream_copy_data(stream, node)

/* Wait to the end of the application with Unix system-call: */
#define SMECY_wait_for_the_end()                                \
  SMECY_PRINT_VERBOSE("Waiting the end of the program\n")       \
  SMECY_IMP_wait_for_the_end()

#endif //SMECY_LIB_H
