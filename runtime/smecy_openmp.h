/* SMECY low-level runtime implementation with OpenMP

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

/* To avoid race conditions in stream loop pipeline: */
#include "conditional_variable_openmp.h"

#ifdef SMECY_VERBOSE
#include <stdio.h>
/* Prefix all the debug messages with "SMECY: " to ease filtering */
#define SMECY_PRINT_VERBOSE_RAW(...)                                    \
  fprintf(stderr, "SMECY: " __VA_ARGS__);                               \
  /* Note that the new line is not atomic because it is in another
     printf... */                                                       \
  fprintf(stderr, "\n");
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


/* SMECY_IMP_ are the real implementations doing the real work, to be
   defined somewhere else. */

/* Implementation macros to deal with mapping and function executions */

// Create a variable name used to pass an argument to function
#define SMECY_IMP_VAR_ARG(func, arg, pe, ...)	\
  p4a_##pe##_##func##_##arg

//  SMECY_CONCATN(SMECY_CONCAT(p4a_##pe##_,SMECY_CONCATENATE(__VA_ARGS__)),##_##func##_##arg)

/* Wrapper that can be used for example to launch the function in another
   thread */
#define SMECY_IMP_LAUNCH_WRAPPER(func_call) func_call

// Implementations for the SMECY library

#define SMECY_IMP_initialize_then_finalize() \
  /* No specific initialization or finalization for the OpenMP
     implementation */


#define SMECY_IMP_set(func, instance, pe, ...) \
  /* Start of the accelerated part */

#define SMECY_IMP_accelerator_end(func, instance, pe, ...)      \
  /* End of the accelerated part */


#define SMECY_IMP_send_arg(func, arg, type, value, pe, ...)        \
  type SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = value

#define SMECY_IMP_cleanup_send_arg(func, arg, type, value, pe, ...)

#define SMECY_IMP_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  SMECY_PRINT_VERBOSE("Sending vector of %zd elements of %s at address" \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"", (size_t) size,      \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)       \
  type* SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = addr

#define SMECY_IMP_cleanup_send_arg_vector(func, arg, type, addr, size, pe, ...) \
  SMECY_PRINT_VERBOSE("Deal with post-sending vector "                  \
                      "of %zd elements of %s at address"                \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"", (size_t) size,      \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)

#define SMECY_IMP_update_arg_vector(func, arg, type, addr, size, pe, ...) \
  type* SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = addr

#define SMECY_IMP_cleanup_update_arg_vector(func, arg, type, addr, size, pe, ...)

#define SMECY_IMP_launch(func, n_args, pe, ...)    \
  SMECY_IMP_launch_##n_args(func, pe, __VA_ARGS__)

#define SMECY_IMP_prepare_get_arg_vector(func, arg, type, addr, size, pe, ...) \
  SMECY_PRINT_VERBOSE("Preparing to receive vector of %zd elements "    \
                      "of %s at address %p from arg #%d of "            \
                      "function \"%s\" on processor \"%s\" n° \"%s\"",  \
                      (size_t) size, #type, addr, arg,                  \
                      #func, #pe, #__VA_ARGS__)                         \
  type* SMECY_IMP_VAR_ARG(func, arg, pe, __VA_ARGS__) = addr

#define SMECY_IMP_get_arg_vector(func, arg, type, addr, size, pe, ...) \
  SMECY_PRINT_VERBOSE("Receiving vector of %zd elements of %s at address" \
                      " %p from arg #%d of function \"%s\" on "         \
                      "processor \"%s\" n° \"%s\"", (size_t) size,      \
                      #type, addr, arg, #func, #pe, #__VA_ARGS__)


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

/* Implementation macros to deal with streaming */

/* There is currently a static limit on the number of streamed loops in a
   program */
#define SMECY_STREAM_NUMBER_MAX 200

/* The algorithm used to do the loop pipelining is the following

   Allocate s data buffers for a pipeline with s stages to move data
   processed through the pipeline.

   stage i

     - ask synchronously a new buffer to stage i-1 or pick cyclicly a new
       buffer in the pool if stage 0

     - work on the data

     - push synchronously the buffer to stage i+1 if not last stage
*/

/* The number of buffers per stage to pass data through the pipeline. For
   example 2 for double buffering, 1 for no buffering.

   Well, right now the implementation is simple and can only cope with
   2. */


/* To globally access to locally allocated buffers and to deal with a
   pipeline stage, indexed by the stream id.

   TODO: This could be a macro inserted at the top-level instead? */
static struct {
  /* Number of stage in this stream: */
  int nb_stages;
  /* A pointer to an array of buffers to be used to transmit information
     in this stream. To be seen as an array
     struct smecy_stream_buffer_type_##stream[nb_stages] */
  void* stream_buffers;
  /* An array of index [nb_stages] of the buffer processed at each
     pipeline stage: */
  int* current_indices;
  /* An array [nb_stages - 1] to synchronize access of producer at this
     stage and comsumer at next stage */
  conditional_variable_t* cv;
} smecy_stream_buffer_global[SMECY_STREAM_NUMBER_MAX];


/* Initialize smecy_stream_buffer_global for a stream */
static void
SMECY_init_pipeline_buffers(int stream,
                            int nb_stages,
                            void *buffers,
			    /* No C99 VLA for C++ compatibility here: */
                            int *current_indices /* [nb_stages] */,
                            conditional_variable_t *cv /* [nb_stages - 1] */) {
  assert(stream < SMECY_STREAM_NUMBER_MAX);
  smecy_stream_buffer_global[stream].nb_stages = nb_stages;
  /* To be globally accessible */
  smecy_stream_buffer_global[stream].stream_buffers = buffers;
  /* Allocate the produce/consume index array: */
  smecy_stream_buffer_global[stream].current_indices = current_indices;
  /* Stage 0 begins to work with buffer 0: */
  smecy_stream_buffer_global[stream].current_indices[0] = 0;
  smecy_stream_buffer_global[stream].cv = cv;
  for (int i = 0; i < nb_stages - 1; i++)
    /* Initialize the monitor to synchronize the producer and the consumer
       between stage i and i+1 */
      conditional_variable_init(&smecy_stream_buffer_global[stream].cv[i]);
}


#define SMECY_IMP_stream_init(stream, nb_stages)                        \
  /* Allocate memory to pass data through each pipeline stage */        \
  struct smecy_stream_buffer_type_##stream smecy_stream_buffer_##stream[nb_stages]; \
  int smecy_stream_buffer_index_##stream[nb_stages];                    \
  conditional_variable_t smecy_stream_cv_##stream[nb_stages - 1];       \
  SMECY_init_pipeline_buffers(stream, nb_stages,                        \
                              (void*)smecy_stream_buffer_##stream,      \
                              smecy_stream_buffer_index_##stream,       \
                              smecy_stream_cv_##stream);                \
  /* Create a parallel block executed by nb_stages threads */           \
  _Pragma(SMECY_STRINGIFY(omp parallel sections num_threads(nb_stages))) \
  SMECY_LBRACE


/* Execute a pipeline stage in its own thread */
#define SMECY_IMP_stream_launch(stream, stage)                          \
  _Pragma("omp section")                                                \
  /* Put the verbose information afterwards to cope with OpenMP         \
     constraints and use. Use a comma to have a single statement. Could be \
     a block, anyway... */                                              \
  SMECY_PRINT_VERBOSE_COMMA("Launched stage %d from stream %d, "        \
                            "OpenMP thread %d of %d", stage, stream,    \
                            omp_get_thread_num(), omp_get_num_threads()) \
  smecy_stream_stage_##stream##_##stage()


/* Initialize the buffer stuff for the output to point on the first buffer
   for this streamed loop for the next stage */
#define SMECY_IMP_stream_get_init_buf(stream, stage)                    \
  /* Each stage will begin with the first buffer anyway */              \
  smecy_stream_buffer_out = &((struct smecy_stream_buffer_type_##stream*)smecy_stream_buffer_global[stream].stream_buffers)[0]; \
  SMECY_PRINT_VERBOSE("\tStage %d of stream %d will use output buffer %p", \
                      stage, stream, smecy_stream_buffer_out)


/* The stage produce something. Push it through the pipeline */
#define SMECY_IMP_stream_put_data(stream, stage)                        \
  /* Release the conditional variable blocking the consumer, when ready to \
     consume */                                                         \
  COND_VAR_NOTIFY_WITH_OP(&smecy_stream_buffer_global[stream]           \
                          .cv[stage],                                   \
                          {                                             \
                            /* Give the current buffer to next stage */ \
                            smecy_stream_buffer_global[stream].current_indices[stage + 1] = smecy_stream_buffer_global[stream].current_indices[stage]; \
                            /* Use the next buffer : */                 \
                            int b = smecy_stream_buffer_global[stream].current_indices[stage]; \
                            b = (b + 1)%smecy_stream_buffer_global[stream].nb_stages; \
                            smecy_stream_buffer_global[stream].current_indices[stage] = b; \
                            /* Next buffer address: */                  \
                            smecy_stream_buffer_out = &((struct smecy_stream_buffer_type_##stream*)smecy_stream_buffer_global[stream].stream_buffers)[b]; \
                            SMECY_PRINT_VERBOSE("\tStage %d of stream %d will use output buffer %p", \
                                                stage, stream, smecy_stream_buffer_out); \
                          })


/* Initialize the buffer input to point on the first buffer
   for this streamed loop for the current stage */
#define SMECY_IMP_stream_get_data(stream, stage)                        \
  /* Get a new data buffer to work on with a synchonous dependency with \
     the producer */                                                    \
  COND_VAR_WAIT_WITH_OP(&smecy_stream_buffer_global[stream]             \
                        .cv[stage - 1],                                 \
                        {                                               \
                          /* Work on the next buffer to be consumed: */     \
                          smecy_stream_buffer_in = &((struct smecy_stream_buffer_type_##stream*)smecy_stream_buffer_global[stream].stream_buffers)[smecy_stream_buffer_global[stream].current_indices[stage]]; \
                          SMECY_PRINT_VERBOSE("\tStage %d of stream %d will use input buffer %p", \
                                              stage, stream, smecy_stream_buffer_in); \
                        })


#define SMECY_IMP_stream_copy_data(stream, stage)               \
  /* Nothing to do since the input and output buffers in a stage are the
     same */
  /* memcpy(smecy_stream_buffer_out, smecy_stream_buffer_in, \
     sizeof(struct smecy_stream_buffer_type_##stream)); */


/* Wait to the end of the application with Unix system-call: */
#define SMECY_IMP_wait_for_the_end()                                    \
  SMECY_RBRACE                                                          \
  /* Insert debugging information here to avoid a parasitic statement in \
     the OpenMP section */                                              \
  SMECY_PRINT_VERBOSE("Waiting for the end of the program")             \
  pause()
