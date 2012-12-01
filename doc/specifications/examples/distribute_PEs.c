#define N 1000
#define NB_CLUSTERS 4
#define NB_PES 16
#define MIN(x, y) ((x) < (y) ? x : y)

#include <stdio.h>

/* Initialize an array between 2 given lines */
void init_array(int a[N][N], int begin, int end) {
  for (int i = begin; i < end; i++)
    for (int j = 0; j < N; j++)
      a[i][j] = 2*i + 3*j;
}

int main() {
  int a[N][N];

  int slice = N/(NB_CLUSTERS*NB_PES);
  /* Launch enough OpenMP thread to control all the fabric:

     Assume that the runtime allows enough threads with nested
     parallelism */
#pragma omp parallel for num_threads(NB_CLUSTERS)
  for (int cluster = 0; cluster < NB_CLUSTERS; cluster++)
#pragma omp parallel for num_threads(NB_PES)
    for (int pe = 0; pe < NB_PES; pe++) {
      /* So now the iterations should be distributed with 1
	 iteration/thread, on NB_CLUSTERS*NB_PES threads.

	 Distribute the initialization on all the fabric: */
      int global_pe = cluster*NB_PES + pe;
      int begin = slice*global_pe;
      int end = MIN(N, slice*(global_pe + 1));
#pragma smecy map(STHORM, cluster, pe)
#pragma smecy arg(a, out, /[begin:end-1][])
      init_array(a, begin, end);
    }

  printf("a[27][42] = %d\n", a[27][42]);

  return 0;
}
