#include <stdio.h>
#include "multiple_compilation_unit.h"

#define N 5
#define M 10


int main() {
  int tab[N][M];
  /* The schedule(static, 1) enforces each iteration to be executed in a
     different thread, whatever the number of CPU is: */
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < N; i++) {
#pragma smecy map(PE,i) \
              arg(1,out,[N][M],/[i][]) \
              arg(2,in)
    init(&tab[i][0], M);
  }

  for (int i = 0; i < N; i++) {
    printf("Line %d :", i);
    for (int j = 0; j < M; j++)
      printf(" %d", tab[i][j]);
    puts("");
  }
  return 0;
}
