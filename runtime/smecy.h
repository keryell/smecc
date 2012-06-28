/* SMECY low-level runtime implementation

   Ronan.Keryell@silkan.com
*/

#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#include <stdio.h>
#ifdef SMECY_VERBOSE
#define SMECY_PRINT_VERBOSE(format, ...) \
  fprintf(stderr, format, __VA_ARGS__);
#else
#define SMECY_PRINT_VERBOSE(format, ...)
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

// Prototypes for the smecy library
#define SMECY_set(pe, instance, func)                                   \
  SMECY_PRINT_VERBOSE("Preparing to launch function \"%s\" on processor \"%s\" n° %d\n", \
                      #func, #pe, instance)                             \
  SMECY_IMP_set(pe, instance, func)

#define SMECY_send_arg(pe, instance, func, arg, type, value)            \
  SMECY_PRINT_VERBOSE("Sending %s to function \"%s\" on processor \"%s\" n° %d\n", \
                      #type, #func, #pe, instance)                      \
  SMECY_IMP_send_arg(pe, instance, func, arg, type, value)

#define SMECY_send_arg_vector(pe, instance, func, arg, type, value, size) \
  SMECY_PRINT_VERBOSE("Sending vector of %zd elements of %s to function \"%s\" on processor \"%s\" n° %d\n", \
                      (size_t) #size, #type, #func, #pe, instance)      \
  SMECY_IMP_send_arg_vector(pe, instance, func, arg, type, value, size)

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
  SMECY_PRINT_VERBOSE("Running function \"%s\" with %zd arguments on processor \"%s\" n° %d\n", \
                      #func, (size_t) n_args, #pe, instance)            \
  SMECY_IMP_launch(pe, instance, func, n_args)
#endif

#define SMECY_get_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Receiving vector of %zd elements of %s at address %p from argument %zd of function \"%s\" on processor \"%s\" n° %d\n", \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_get_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_future_get_arg_vector(pe, instance, func, arg, type, addr, size) \
  SMECY_PRINT_VERBOSE("Preparing to receiving vector of %zd elements of %s at address %p from argument %zd of function \"%s\" on processor \"%s\" n° %d\n", \
                      (size_t) size, #type, addr, arg, #func, #pe, instance) \
  SMECY_IMP_future_get_arg_vector(pe, instance, func, arg, type, addr, size)

#define SMECY_get_return(pe, instance, func, type)                      \
  SMECY_PRINT_VERBOSE("Returning %s from function \"%s\" on processor \"%s\" n° %d\n", \
                      #type, #func, #pe, instance)                      \
  SMECY_IMP_get_return(pe, instance, func, type)


void square_symmetry_smecy(int width, int height, int* image,
                     int square_size, int x_offset, int y_offset) ;

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

#endif //SMECY_LIB_H
