#ifndef SMECY_LIB_H
#define SMECY_LIB_H

#include <stdio.h>

#define SMECY_set(pe, instance, func)										printf("Mapping function %s to %s n°%d\n",#func,#pe,instance)
#define SMECY_send_arg(pe, instance, func, arg, type, value)				printf("Sending argument to function %s on %s n°%d\n",#func,#pe,instance)
#define SMECY_send_arg_vector(pe, instance, func, arg, type, value, size)	printf("Sending argument vector to function %s on %s n°%d\n",#func,#pe,instance)
#define SMECY_launch(pe, instance, func)									printf("Launching function %s to %s n°%d\n",#func,#pe,instance)
#define SMECY_get_arg_vector(pe, instance, func, arg, type, value, size)	printf("Getting argument vector to function %s on %s n°%d\n",#func,#pe,instance)
#define SMECY_get_return(pe, instance, func, type)							printf("Getting return of function %s on %s n°%d\n",#func,#pe,instance)

#endif //SMECY_LIB_H
