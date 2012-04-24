#include <stdio.h>

/* Generate counting numbers */
void generate(int * thing) {
  static int state = 0;
  *thing = state++;
}


/* Some basic transformation */
void transform(int * thing, int product) {
  *thing *= product;
}

/* Consume the flux */
void consume(int * thing) {
  printf("%d ", *thing);
}


int main() {
  int b[1] = { sizeof(int) };
  int c = 3;

#pragma smecy stream_loop
  while (1) {
#pragma smecy stream_node(1) arg(1,out) map(PE,1)
    generate(b);
#pragma smecy stream_node(2) arg(1,inout)
    transform(b, c);
#pragma smecy stream_node(3) arg(1,in)
    consume(b);
  }

  return 0;
}
