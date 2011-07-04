#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#include <stdio.h>

#define SMECY_set(pe, instance, func)										printf("Preparing to launch %s on %s n°%d\n",#func,#pe,instance)
#define SMECY_send_arg(pe, instance, func, arg, type, value)				type pe##_##instance##_##func##_##arg = value
#define SMECY_send_arg_vector(pe, instance, func, arg, type, value, size)	type* pe##_##instance##_##func##_##arg = (int*)value
#if 0
	#define SMECY_launch(pe, instance, func)								{ if (#func == "square_symmetry")\
																			   square_symmetry_smecy( pe##_##instance##_square_symmetry_1, \
																			   pe##_##instance##_square_symmetry_2, \
																			   (int*)pe##_##instance##_square_symmetry_3, \
																			   pe##_##instance##_square_symmetry_4, \
																			   pe##_##instance##_square_symmetry_5, \
																			   pe##_##instance##_square_symmetry_6); \
																			}
#else
	#define SMECY_launch(pe, instance, func)
#endif 
#define SMECY_get_arg_vector(pe, instance, func, arg, type, value, size)	printf("arg\n");
#define SMECY_get_return(pe, instance, func, type)							printf("Getting return of function %s on %s n°%d\n",#func,#pe,instance)

#endif //SMECY_LIB_H

void square_symmetry_smecy(int width, int height, int* image,
		     int square_size, int x_offset, int y_offset) ;
		     

