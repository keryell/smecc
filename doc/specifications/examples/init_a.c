#define N 1000
#include <stdio.h>

void init_array(int a[N][N]) {
  /* The OpenMP part is understood by smecc/Par4All to generate 2D OpenCL
     workitems */
#pragma omp parallel for
  for (int i = 0; i < N; i++)
#pragma omp parallel for
    for (int j = 0; j < N; j++)
      a[i][j] = 2*i + 3*j;
}

int main() {
  int a[N][N];

#pragma smecy map(OpenCL) arg(a, out)
  init_array(a);

  printf("a[27][42] = %d\n", a[27][42]);

  return 0;
}
