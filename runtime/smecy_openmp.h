/* SMECY low-level runtime implementation with OpenMP

   Ronan.Keryell@silkan.com
*/

/* SMECY_IMP_ are the real implementations doing the real work, to be
   defined somewhere else. */

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

#define SMECY_IMP_send_arg_vector(pe, instance, func, arg, type, addr, size) \
  type* SMECY_IMP_VAR_ARG(pe, instance, func, arg) = addr

#define SMECY_IMP_launch(pe, instance, func, n_args)    \
  SMECY_IMP_launch_##n_args(pe, instance, func)

#define SMECY_IMP_get_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_IMP_VAR_ARG(pe, instance, func, arg)

#define SMECY_IMP_future_get_arg_vector(pe, instance, func, arg, type, addr, size) \
  type* SMECY_IMP_VAR_ARG(pe, instance, func, arg) = addr

#define SMECY_IMP_get_return(pe, instance, func, type)

/* Implementation of the function calls themselves */
#define SMECY_IMP_launch_0(pe, instance, func)  \
  SMECY_IMP_LAUNCH_WRAPPER(func())


#define SMECY_IMP_launch_1(pe, instance, func)                          \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_VAR_ARG(pe, instance, func, 1)))

#define SMECY_IMP_launch_2(pe, instance, func)                          \
  SMECY_IMP_LAUNCH_WRAPPER(func(SMECY_IMP_VAR_ARG(pe, instance, func, 1), \
                                SMECY_IMP_VAR_ARG(pe, instance, func, 2)))

