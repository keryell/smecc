#define N 1000
#define NB_CLUSTERS 4
#define NB_PES 16
#define MIN(x, y) ((x) < (y) ? x : y)

#include <stdio.h>

/* Initialize an array between 2 given lines */
void init_array(int a[N][N]) {
#pragma omp parallel for
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      a[i][j] = 2*i + 3*j;
}

void mult(int a[N][N], int begin, int end, int fact) {
  for (int i = begin; i < end; i++)
    for (int j = 0; j < N; j++)
      a[i][j] *= fact;
}

// Compute a distribution by stripe on the PEs:
#define ITER_BEGIN N/(NB_CLUSTERS*NB_PES)*(cluster*NB_PES + pe)
#define ITER_END MIN(N, N/(NB_CLUSTERS*NB_PES)*(cluster*NB_PES + pe + 1))

int main() {
  int a[N][N];

  // Initialize in parallel on the Cortex A9:
  init_array(a);

  /* Then do some computations with the fabric

     Launch enough OpenMP thread on the Cortex A9 to control all the
     clusters */
#pragma omp parallel for num_threads(NB_CLUSTERS)
  for (int cluster = 0; cluster < NB_CLUSTERS; cluster++)
    switch(cluster) {
    case 0:
      // The cluster 0:
#pragma omp parallel for num_threads(NB_PES)
    for (int pe = 0; pe < NB_PES; pe++)
#pragma smecy map(STHORM, cluster, pe)
#pragma smecy arg(a, inout, /[begin:end-1][])
      mult(a, ITER_BEGIN, ITER_END, -1);
    break;

    case 1:
      // The cluster 1:
#pragma omp parallel for num_threads(NB_PES)
    for (int pe = 0; pe < NB_PES; pe++)
#pragma smecy map(STHORM, cluster, pe)
#pragma smecy arg(a, inout, /[begin:end-1][])
      mult(a, ITER_BEGIN, ITER_END, 3);
    break;

    case 2:
      // The cluster 2:
#pragma omp parallel for num_threads(NB_PES)
    for (int pe = 0; pe < NB_PES; pe++)
#pragma smecy map(STHORM, cluster, pe)
#pragma smecy arg(a, inout, /[begin:end-1][])
      mult(a, ITER_BEGIN, ITER_END, -7);
    break;

    case 3:
      // The cluster 3:
#pragma omp parallel for num_threads(NB_PES)
    for (int pe = 0; pe < NB_PES; pe++)
#pragma smecy map(STHORM, cluster, pe)
#pragma smecy arg(a, inout, /[begin:end-1][])
      mult(a, ITER_BEGIN, ITER_END, 9);
    break;
    }

  printf("a[27][42] = %d\n", a[27][42]);

  return 0;
}
