#define N 1000
#define NB_CLUSTERS 4
#define NB_PES 16
#define MIN(x, y) ((x) < (y) ? x : y)

#include <stdio.h>

/* Initialize an array between 2 given lines */
void init_array(int a[N][N], int begin, int end) {
#pragma omp parallel for
  for (int i = begin; i < end; i++)
#pragma omp parallel for
    for (int j = 0; j < N; j++)
      a[i][j] = 2*i + 3*j;
}

int main() {
  int a[N][N];

  int slice = N/(NB_CLUSTERS*NB_PES);
  // Launch enough OpenMP thread to control all the fabric:
#pragma omp parallel for
  for (int cluster = 0; cluster < NB_CLUSTERS; cluster++)
#pragma omp parallel for
    for (int pe = 0; pe < NB_PES; pe++) {
      // Distribute the initialization on all the fabric
      int global_pe = cluster*NB_PES + pe;
      int begin = slice*global_pe;
      int end = MIN(N, slice*(global_pe + 1));
#pragma smecy map(STHORM, cluster, pe)
#pragma smecy arg(a, out, /[begin:end-1][])
      init_array(a, begin, end);

  printf("a[27][42] = %d\n", a[27][42]);

  return 0;
}
