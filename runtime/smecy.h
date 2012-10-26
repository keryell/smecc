/* SMECY low-level runtime implementation

   Ronan.Keryell@silkan.com
*/

#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#ifdef SMECY_VERBOSE
#include <stdio.h>
#define SMECY_PRINT_VERBOSE_RAW(...)                                    \
  fprintf(stderr, __VA_ARGS__)
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

/* Some helper function to redirect a macro call to M21 for example when M
   is called with 21 arguments

   Inspired from
   http://stackoverflow.com/questions/3868289/count-number-of-parameters-in-c-variable-argument-method-call
   as already used in Par4All src/p4a_accel/p4a_accel-OpenCL.h

   The use case is then
   #define M(...) SMECY_CONCATN(M,SMECY_NARG(__VA_ARGS__))(0,__VA_ARGS__)
*/
#define SMECY_NARG(...) SMECY_NARG_(__VA_ARGS__,SMECY_RSEQ_N())

/** Interprets the preceding list as a unique __VA_ARGS__
 */

#define SMECY_NARG_(...) SMECY_ARG_N(__VA_ARGS__)


/** Positional parameters are expanded before inserting the macro
    expansion.  The expansion of parameter list followed by the reverse
    enumeration results in the returned value N that equals the exact
    number of parameters.
*/
#define SMECY_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N

/** A reverse enumeration used to compute the argument number up to 63.
 */
#define SMECY_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

/** When the number of arguments is known, automatic call by
    concatenation to the function SMECY_argn, where n is the number of
    arguments.
 */
#define SMECY_argN(...) SMECY_CONCATN(SMECY_arg,SMECY_NARG(__VA_ARGS__))(0,__VA_ARGS__)

/** Forcing the interpretation of two fields that have to be concatenated.
 */
#define SMECY_CONCATN(a,b) SMECY_CONCAT(a,b)

/** Concatenation of two fields
 */
#define SMECY_CONCAT(a,b) a ## b

/* Define some variadic helper macros:

   Concatenate arguments separated by '_'

   The variadic API
*/
#define SMECY_CONCATENATE(...) SMECY_CONCATN(SMECY_CONCATENATE,SMECY_NARG(__VA_ARGS__))(__VA_ARGS__)
/* And now few implementations: */
#define SMECY_CONCATENATE0()
#define SMECY_CONCATENATE1(x) x
#define SMECY_CONCATENATE2(x,y) x##_##y
#define SMECY_CONCATENATE3(x,y,z) x##_##y##_##z
#define SMECY_CONCATENATE4(x,y,z,t) x##_##y##_##z##_##t


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

/* According to the targets, the PE coordinates have 0 or more dimension,
   so use __VA_ARGS__ for them at the end of the macros */

#define SMECY_set(func, pe, ...)					\
  SMECY_PRINT_VERBOSE("Preparing to launch function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"\n",                   \
                      #func, #pe, #__VA_ARGS__)                         \
  SMECY_IMP_set(func, pe, __VA_ARGS__)

#define SMECY_send_arg(func, arg, type, value, pe, ...)             \
  SMECY_PRINT_VERBOSE("Sending %s as arg #%d to function \"%s\""    \
                      " on processor \"%s\" n° \"%s\"\n",           \
                      #type, arg, #func, #pe, #__VA_ARGS__)         \
  SMECY_IMP_send_arg(func, arg, type, value, pe, __VA_ARGS__)

#define SMECY_cleanup_send_arg(func, arg, type, value, pe, ...)		\
  SMECY_PRINT_VERBOSE("Deal with post-sending %s as arg #%d"            \
                      " to function \"%s\" on processor \"%s\" n° "     \
                      "\"%s\"\n", #type, arg, #func, #pe, #__VA_ARGS__) \
  SMECY_IMP_cleanup_send_arg(func, arg, type, value, pe, __VA_ARGS__)

#define SMECY_send_arg_vector(func, arg, type, addr, size, pe, ...)	\
  SMECY_PRINT_VERBOSE("Sending vector of %zd elements of %s at address" \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"\n", (size_t) size,    \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)       \
  SMECY_IMP_send_arg_vector(func, arg, type, addr, size, pe, __VA_ARGS__)

#define SMECY_cleanup_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  SMECY_PRINT_VERBOSE("Deal with post-sending vector "                  \
                      "of %zd elements of %s at address"                \
                      " %p from arg #%d of function \"%s\" on "          \
                      "processor \"%s\" n° \"%s\"\n", (size_t) size,    \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)       \
  SMECY_IMP_cleanup_send_arg_vector(func, arg, type, addr,              \
                                    size, pe, __VA_ARGS__)

#define SMECY_update_arg_vector(func, arg, type, addr, size, pe, ...)	\
  SMECY_PRINT_VERBOSE("Update by sending vector "                       \
                      "of %zd elements of %s at address"                \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"\n", (size_t) size,    \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)       \
  SMECY_IMP_update_arg_vector(func, arg, type, addr, size, pe, __VA_ARGS__)

#define SMECY_cleanup_update_arg_vector(func, arg, type, addr, size, pe, ...) \
  SMECY_PRINT_VERBOSE("Udate by receiving vector "                      \
                      "of %zd elements of %s at address"                \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"\n",(size_t) size,     \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)       \
  SMECY_IMP_cleanup_update_arg_vector(func, arg, type, addr, size, pe, ...)

#define SMECY_launch(func, n_args, pe, ...)				\
  SMECY_PRINT_VERBOSE("Running function \"%s\" with %zd arguments on "  \
                      "processor \"%s\" n° \"%s\"\n",                   \
                      #func, (size_t) n_args, #pe, #__VA_ARGS__)	\
  SMECY_IMP_launch(func, n_args, pe, __VA_ARGS__)

#define SMECY_prepare_get_arg_vector(func, arg, type, addr, size, pe, ...) \
  SMECY_PRINT_VERBOSE("Preparing to receiving vector of %zd elements "  \
                      "of %s at address %p from arg #%d of "            \
                      "function \"%s\" on processor \"%s\" n° \"%s\"\n", \
                      (size_t) size, #type, addr, arg,                  \
                      #func, #pe, #__VA_ARGS__)                         \
  SMECY_IMP_prepare_get_arg_vector(func, arg, type, addr, size, pe, __VA_ARGS__)

#define SMECY_get_arg_vector(func, arg, type, addr, size, pe, ...)	\
  SMECY_PRINT_VERBOSE("Receiving vector of %zd elements of %s at address" \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"\n", (size_t) size,    \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)       \
  SMECY_IMP_get_arg_vector(func, arg, type, addr, size, pe, __VA_ARGS__)

#define SMECY_get_return(func, type, pe, ...)				\
  SMECY_PRINT_VERBOSE("Returning %s from function \"%s\" on processor"  \
                      " \"%s\" n° \"%s\"\n", #type, #func, #pe, #__VA_ARGS__) \
  SMECY_IMP_get_return(func, type, pe, __VA_ARGS__)



/* Interface macros to deal with streaming */

#define SMECY_stream_init(stream, nbstreams)                            \
  SMECY_PRINT_VERBOSE("Init stream %d with %d stages\n", stream, nbstreams) \
  SMECY_IMP_stream_init(stream, nbstreams)

#define SMECY_stream_launch(stream, stage)                              \
  /* Put the verbose information afterwards inside the implementation   \
     to cope with OpenMP constraints */                                 \
  SMECY_IMP_stream_launch(stream, stage)

#define SMECY_stream_get_init_buf(stream, stage)                        \
  SMECY_PRINT_VERBOSE("Init get buffer on stage %d of stream %d\n",     \
                      stage, stream)                                    \
  SMECY_IMP_stream_get_init_buf(stream, stage)

#define SMECY_stream_put_data(stream, stage)                            \
  SMECY_PRINT_VERBOSE("Post data for next stage on stage %d of stream %d\n", \
                      stage, stream)                                    \
  SMECY_IMP_stream_put_data(stream, stage);                             \
  SMECY_PRINT_VERBOSE("Sent the data for next stage on stage %d of stream %d\n", \
                      stage, stream)                                    \

#define SMECY_stream_get_data(stream, stage)                            \
  SMECY_PRINT_VERBOSE("Get data from previous stage on stage %d"        \
                      " of stream %d\n", stage, stream)                 \
  SMECY_IMP_stream_get_data(stream, stage);                             \
  SMECY_PRINT_VERBOSE("Got the data from previous stage on stage %d"    \
                      " of stream %d\n", stage, stream)                 \

#define SMECY_stream_copy_data(stream, stage)                           \
  SMECY_PRINT_VERBOSE("Copy data from previous stage to next stage unchanged" \
                      " on stage %d of stream %d\n", stage, stream)     \
  SMECY_IMP_stream_copy_data(stream, stage)

/* Wait for the end of the application with Unix system-call: */
#define SMECY_wait_for_the_end()                                        \
  /* Put the verbose information afterwards inside the implementation   \
     to cope with OpenMP constraints */                                 \
  SMECY_IMP_wait_for_the_end()

#endif //SMECY_LIB_H
