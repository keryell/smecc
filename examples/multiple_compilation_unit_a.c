#include "multiple_compilation_unit.h"

int main() {
  int tab[10][100];
  /* The schedule(static, 1) enforces each iteration to be executed in a
     different thread, whatever the number of CPU is: */
  //#pragma omp parallel for schedule(static, 1)
  for (int i=0; i<10; i++) {
#pragma smecy map(PE,i) \
              arg(1,out,[10][100],/[i][]) \
              arg(2,in)
    init(&tab[i][0],100);
  }
  return 0;
}
