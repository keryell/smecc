/* OpenMP implementation of conditional variable for 2 threads

   This is a simplification of the general conditional variable monitor
   since there is no queue to deal with more than one blocked thread

   Ronan.Keryell@silkan.com
*/

#include <omp.h>

/* 2 locks to phase the work flow through the conditional variable */
typedef struct {
  /* Control if we are ready to accept notifications, that is we are not
     still dealing with a previous notification */
  omp_lock_t ready_for_notify_lock;
  /* To block the waiting thead up to the notification */
  omp_lock_t notify_lock;
} conditional_variable_t;


/* Initialize the synchronization structure */
static inline void conditional_variable_init(conditional_variable_t* nw) {
  // Classical OpenMP lock initialization
  omp_init_lock(&nw->notify_lock);
  omp_init_lock(&nw->ready_for_notify_lock);
  // Prepare to wait since there is no notify yet
  omp_set_lock(&nw->notify_lock);
}

#define COND_VAR_NOTIFY_WITH_OP(nw, ...)                \
do {                                                    \
  /* Wait for the consumption of the previous notify */ \
  omp_set_lock(&nw->ready_for_notify_lock);             \
  __VA_ARGS__;                                          \
  /* Release the waiting thread */                      \
  omp_unset_lock(&nw->notify_lock);                     \
 } while(0)


#define COND_VAR_WAIT_WITH_OP(nw, ...)          \
do {                                            \
  /* Wait for a notification */                 \
  omp_set_lock(&nw->notify_lock);               \
  __VA_ARGS__;                                  \
  /* Allow for a future notification */         \
  omp_unset_lock(&nw->ready_for_notify_lock);   \
} while(0)
