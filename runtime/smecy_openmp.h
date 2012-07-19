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

/* As in LaTeX to allow adding unbalanced {} and to avoid messing up
   automatic indentation */
#define SMECY_LBRACE {
#define SMECY_RBRACE }

/* Expansion hack to give a full string to _Pragma() */
#define SMECY_STRINGIFY(s) #s

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

#define SMECY_IMP_cleanup_send_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_IMP_update_arg_vector(pe, instance, func, arg, type, addr, size) \
  type* SMECY_IMP_VAR_ARG(pe, instance, func, arg) = addr

#define SMECY_IMP_cleanup_update_arg_vector(pe, instance, func, arg, type, addr, size)

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

/* There is currently a static limit on the number of streamed loops in a
   program */
#define SMECY_STREAM_NUMBER_MAX 200

/* The algorithm used to do double buffering is

   stage i locks data input
   stage i locks data output
   stage i does the copy from input to output to implement any write-through
   stage i does the computation
   stage i unlocks data input
   stage i unlocks data output
   stage i does its double buffer index flip flop for next iteration
*/

/* The number of buffers per stage to pass data through the pipeline. For
   example 2 for double buffering, 1 for no buffering.

   Well, right now the implementation is simple and can only cope with
   2. */
#define SMECY_STREAM_STAGE_BUFFER_NB 2

/* To deal with a pipeline stage */
typedef struct {
  /* The index of the last buffer consumed */
  int consume;
  /* The index of the next index buffer to be produced */
  int produce;
  /* Synchronize access of producer at stage i and comsumer at stage i-1 */
  conditional_variable_t cv;
} SMECY_stage_control_t;

/* To globally access to locally allocated buffers, indexed by the stream
   id.

   TODO: This could be a macro inserted at the top-level instead? */
static struct {
  /* A pointer to an array of buffers to be used to transmit information
     in this stream. To be seen as an array struct
     smecy_stream_buffer_type_##stream
     [nb_stages-1][SMECY_STREAM_STAGE_BUFFER_NB] */
  void* stream_buffers;
  /* An array of produce/consume indices to be used to deal with
     multi-buffering at each pipeline stage in this stream with a monitor
     to synchronize the consumer and producer: */
  SMECY_stage_control_t* current_indices;
} smecy_stream_buffer_global[SMECY_STREAM_NUMBER_MAX];


/* Initialize smecy_stream_buffer_global for a stream */
static void
SMECY_init_pipeline_buffers(int stream,
                            int nbstreams,
                            void *buffers) {
  assert(stream < SMECY_STREAM_NUMBER_MAX);
  /* To be globally accessible */
  smecy_stream_buffer_global[stream].stream_buffers = buffers;
  /* Allocate the produce/consume index array: */
  smecy_stream_buffer_global[stream].current_indices = (SMECY_stage_control_t*)
    malloc(sizeof(SMECY_stage_control_t)*(nbstreams - 1));
  for (int i = 0; i < nbstreams - 1; i++) {
    /* Next buffer to write: */
    smecy_stream_buffer_global[stream].current_indices[i].produce = 0;
    /* Last buffer read. Since it is an initialization, it prepare to read
       buffer 0 once it has been produced: */
    smecy_stream_buffer_global[stream].current_indices[i].consume =
      SMECY_STREAM_STAGE_BUFFER_NB - 1;
    /* Initialize the monitor to synchronize the producer and the consumer */
      conditional_variable_init(&smecy_stream_buffer_global[stream]
                                .current_indices[i].cv);
  }
}


#define SMECY_IMP_stream_init(stream, nb_stages)                        \
  /* Allocate memory to pass data through each pipeline stage in a double \
     buffer way */                                                      \
  struct smecy_stream_buffer_type_##stream smecy_stream_buffer_##stream[nb_stages-1][SMECY_STREAM_STAGE_BUFFER_NB]; \
  SMECY_init_pipeline_buffers(stream, nb_stages,                        \
                              (void*)smecy_stream_buffer_##stream);     \
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
                            "OpenMP thread %d of %d\n", stage, stream,  \
                            omp_get_thread_num(), omp_get_num_threads()) \
  smecy_stream_stage_##stream##_##stage()


/* Initialize the buffer stuff for the output to point on the first buffer
   for this streamed loop for the next stage */
#define SMECY_IMP_stream_get_init_buf(stream, stage)                    \
/* Verify the initialization by SMECY_IMP_stream_init() */              \
  assert(smecy_stream_buffer_global[stream].current_indices[stage].produce == 0); \
  /* Since SMECY_stream_put_data() is at the end of the pipeline stage, we \
     need a buffer to write the result before: */                       \
  smecy_stream_buffer_out = &((struct smecy_stream_buffer_type_##stream*)smecy_stream_buffer_global[stream].stream_buffers)[stage*SMECY_STREAM_STAGE_BUFFER_NB + smecy_stream_buffer_global[stream].current_indices[stage].produce /* <- Should be 0 */]


/* The stage produce something. Push it through the pipeline */
#define SMECY_IMP_stream_put_data(stream, stage)                                \
  /* Release the conditional variable blocking the consumer, when ready to \
     consume */                                                         \
  COND_VAR_NOTIFY_WITH_OP(&smecy_stream_buffer_global[stream]           \
                          .current_indices[stage].cv,                   \
                          {                                             \
                            /* Deal with the multiple buffering here.   \
                               Use the next buffer for the producer */  \
                            smecy_stream_buffer_global[stream].current_indices[stage].produce = (smecy_stream_buffer_global[stream].current_indices[stage].produce + 1)%SMECY_STREAM_STAGE_BUFFER_NB; \
                            /* Update the pointer to the buffer to use as \
                               output: */                               \
                            smecy_stream_buffer_out = &((struct smecy_stream_buffer_type_##stream*)smecy_stream_buffer_global[stream].stream_buffers)[stage*SMECY_STREAM_STAGE_BUFFER_NB + smecy_stream_buffer_global[stream].current_indices[stage].produce]; \
                              })


/* Initialize the buffer input to point on the first buffer
   for this streamed loop for the current stage */
#define SMECY_IMP_stream_get_data(stream, stage)                         \
  /* Get a new data buffer to work on with a synchonous dependency with \
     the producer */                                                    \
  COND_VAR_WAIT_WITH_OP(&smecy_stream_buffer_global[stream]             \
                        .current_indices[stage - 1].cv,                 \
                        {                                               \
                          /* Use the next buffer to be consumed: */     \
                          smecy_stream_buffer_global[stream].current_indices[stage - 1].consume = (smecy_stream_buffer_global[stream].current_indices[stage - 1].consume + 1)%SMECY_STREAM_STAGE_BUFFER_NB; \
                          /* Update the pointer to the buffer to use as \
                             input: */                                  \
                          smecy_stream_buffer_in = &((struct smecy_stream_buffer_type_##stream*)smecy_stream_buffer_global[stream].stream_buffers)[stage*SMECY_STREAM_STAGE_BUFFER_NB + smecy_stream_buffer_global[stream].current_indices[stage - 1].consume]; \
                        })


#define SMECY_IMP_stream_copy_data(stream, stage)               \
  memcpy(smecy_stream_buffer_out, smecy_stream_buffer_in,       \
         sizeof(struct smecy_stream_buffer_type_##stream));


/* Wait to the end of the application with Unix system-call: */
#define SMECY_IMP_wait_for_the_end()                                    \
  SMECY_RBRACE                                                          \
  /* Insert debugging information here to avoid a parasitic statement in \
     the OpenMP section */                                              \
  SMECY_PRINT_VERBOSE("Waiting the end of the program\n")               \
  pause()
