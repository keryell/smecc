#include <stdio.h>

/* Generate counting numbers */
void generate(int * thing, int * product) {
  static int state = 0;
  *thing = state++;
  *product = 3;
  printf("\t*** Generate: %d (%p)\n", *thing, thing);
}


/* Some basic transformation */
void transform(int * thing, int * product) {
  *thing *= *product;
  printf("\tvvv Transform: %d (%p) with %d (%p)\n", *thing, thing, *product, product);
}

/* Consume the flux */
void consume(int * thing) {
  printf("\t>>> Consume: %d (%p)\n", *thing, thing);
}


int main() {
  int b[1] = { sizeof(int) };
  int c[1] = { 3 };

#pragma smecy stream_loop
  while (1) {
#pragma smecy stage arg(1,out) arg(2,out) map(PE,1)
    generate(b, c);
#pragma smecy stage label(2) arg(1,inout) arg(2,in)
    transform(b, c);
#pragma smecy stage arg(1,in)
    consume(b);
  }

  return 0;
}
