#include <iostream>

void produce(double* v) {
  static int seed = 0;
  *v = seed++;
}

void scale(double* v, int factor) {
  *v *= factor;
}

void display(double* v) {
  std::cout << *v << " ";
}


int main() {
  double value[1];
  int c = 3;
#pragma smecy stream_loop
  while (1) {
#pragma smecy stream_node(1) arg(1,out) map(PE,1)
      produce(value);
#pragma smecy stream_node(2) arg(1,inout) map(CPU, 5)
      scale(value, c);
#pragma smecy stream_node(2) arg(1,in) map(GPU, 46)
      display(value);
	}

  return 0;
}
