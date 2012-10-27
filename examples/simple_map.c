#include <stdio.h>

#define N 10
#define M 5

void init(int* array, int size, int scale) {
  for (int i = 0; i < size; i++)
    array[i] = i*scale;
}

int main() {
  int tab[N][M];
  /* The schedule(static, 1) enforces each iteration to be executed in a
     different thread, whatever the number of CPU is: */
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < N; i++) {
    // Map on STHORM cluster 0 PE i:
#pragma smecy map(STHORM, 0, i)	       \
              arg(1,out,[N][M],/[i][]) \
              arg(2,in) \
              arg(3,in)
    init(&tab[i][0], M, i+1);
  }

  for (int i = 0; i < N; i++) {
    printf("Line %d :", i);
    for (int j = 0; j < M; j++)
      printf("%d ", tab[i][j]);
    puts("");
  }
  return 0;
}
