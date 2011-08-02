#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#include <stdio.h>

//prototypes for the smecy library
//note that it can only work if the code maps only the square_symmetry function
#define SMECY_set(pe, instance, func)										printf("Preparing to launch %s on %s n°%d\n",#func,#pe,instance)
#define SMECY_send_arg(pe, instance, func, arg, type, value)				type pe##_##func##_##arg = value
#define SMECY_send_arg_vector(pe, instance, func, arg, type, value, size)	type* pe##_##func##_##arg = (int*)value
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
	#define SMECY_launch(pe, instance, func)
#endif
#define SMECY_get_arg_vector(pe, instance, func, arg, type, value, size)	printf("arg\n");
#define SMECY_get_return(pe, instance, func, type)							printf("Getting return of function %s on %s n°%d\n",#func,#pe,instance)

void square_symmetry_smecy(int width, int height, int* image,
		     int square_size, int x_offset, int y_offset) ;

// RK: je ne suis pas sûr que cette histoire de DbLink devrait apparaître
// Pourquoi int ?

//prototypes for the stream library
typedef int DbLink;

DbLink pth_CreateDbLink(int size);
void* DbLinkGetInitBuf(DbLink outputLink);
void* DbLinkGetData(DbLink inputLink);
void* DbLinkPutData(DbLink inputLink);
int pth_CreateProcess(int (*f)(), ...);
void pause();

#endif //SMECY_LIB_H
