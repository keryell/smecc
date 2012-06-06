#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#include <stdio.h>
#ifdef SMECY_VERBOSE
#define SMECY_PRINT_VERBOSE(format, ...) \
  fprintf(stderr, format, __VA_ARGS__);
#else
#define SMECY_PRINT_VERBOSE(format, ...)
#endif

//prototypes for the smecy library
//note that it can only work if the code maps only the square_symmetry function
#define SMECY_set(pe, instance, func) \
  SMECY_PRINT_VERBOSE("Preparing to launch function \"%s\" on processor %s n° %d\n", \
		      #func, #pe, instance);

#define SMECY_send_arg(pe, instance, func, arg, type, value) \
  SMECY_PRINT_VERBOSE("Sending %s to function \"%s\" on processor %s n° %d\n", \
		      #type, #func, #pe, instance) \
  type pe##_##instance##_##func##_##arg = value

#define SMECY_send_arg_vector(pe, instance, func, arg, type, value, size) \
  SMECY_PRINT_VERBOSE("Sending vector of %zd elements of %s to function \"%s\" on processor %s n° %d\n", \
		      #size, #type, #func, #pe, instance)		\
	type* pe##_##instance##_##func##_##arg = (int*)value
#if 0
	#define SMECY_launch(pe, instance, func)								{ if (#func == "square_symmetry")\
																			   square_symmetry_smecy( pe##_square_symmetry_1, \
																			   pe##_square_symmetry_2, \
																			   (int*)pe##_square_symmetry_3, \
																			   pe##_square_symmetry_4, \
																			   pe##_square_symmetry_5, \
																			   pe##_square_symmetry_6); \
																			}
#else
	#define SMECY_launch(pe, instance, func) \
	  SMECY_PRINT_VERBOSE("Running function \"%s\" on processor %s n° %d\n", \
			      #func, #pe, instance);
#endif

#define SMECY_get_arg_vector(pe, instance, func, arg, type, value, size) \
	SMECY_PRINT_VERBOSE("Receiving vector of %zd elements of %s from function \"%s\" on processor %s n° %d\n", \
			    #size, #type, #func, #pe, instance)

#define SMECY_get_return(pe, instance, func, type) \
	SMECY_PRINT_VERBOSE("Returning %s from function \"%s\" on processor %s n° %d\n", \
		      #type, #func, #pe, instance)


void square_symmetry_smecy(int width, int height, int* image,
		     int square_size, int x_offset, int y_offset) ;

// RK: je ne suis pas sûr que cette histoire de DbLink devrait apparaître
// Pourquoi int ?

//prototypes for the stream library
typedef int DbLink;

DbLink pth_CreateDbLink(int size) { return (DbLink)0; }
void* DbLinkGetInitBuf(DbLink outputLink) { return NULL; }
void* DbLinkGetData(DbLink inputLink) { return NULL; }
void* DbLinkPutData(DbLink inputLink) { return NULL; }
int pth_CreateProcess(int (*f)(), ...) {return 0;}

#endif //SMECY_LIB_H
