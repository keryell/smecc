#ifndef SMECY_H
#define SMECY_H

#include <stdio.h>

#define SMECY_set(pe, instance, func) printf("youpi")
#define SMECY_launch(pe, instance, func) printf("youpla")
#define SMECY_send_arg(pe, instance, func, arg, type, value) printf("youplaboum")
#define SMECY_get_arg(pe, instance, func, arg, type, value) printf("tsoin tsoin")
#define SMECY_send_arg_vector(pe, instance, func, arg, type, value, size) printf("youplaboum !")
#define SMECY_get_arg_vector(pe, instance, func, arg, type, value, size) printf("tsoin tsoin !")

#endif //SMECY_H
