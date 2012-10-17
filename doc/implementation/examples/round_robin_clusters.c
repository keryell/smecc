#define N 10
#define NB_CLUSTERS 4
#define NB_PES 16
#define MIN(x, y) ((x) < (y) ? x : y)

#include <stdio.h>


int get_ticket() {
  static int t = 0;
  int ticket;
  // In case this is called from several threads. Avoid a flush, anyway
#pragma omp atomic capture
  ticket = t++;
  return ticket;
}


int get_data(t) {
  //sleep(t&1);
  /* In a real application, get a radar signal time slice for example */
  return t*2;
}


int compute(int d, int cluster, int pe) {
  /* In a real application, do a computation on the data */
  return d*cluster + pe;
}



int main() {
  /* Launch all the threads to control the clusters only once */
#pragma omp parallel num_threads(NB_CLUSTERS)
  {
    for (int i = 0; i < N; i++) {
      /* Execute 1 iteration per thread and there will be some ordered
	 statement.

	 It is useless to wait at the end of the iterations, but it looks
	 like a nowait here break the ordered. Compiler bug? */
#pragma omp for schedule(static, 1) ordered
      for (int cluster = 0; cluster < NB_CLUSTERS; cluster++) {
	/* Get an ID in the order of the sequential iteration. Remove the
	   ordered if it is not needed. */
	int t;
#pragma omp ordered
	{
	  t = get_ticket();
	}
	int d = get_data(t);
	int r;
#pragma omp parallel for num_threads(NB_PES) reduction(+:r)
	for (int pe = 0; pe < NB_PES; pe++)
#pragma smecy map(STHORM, cluster, pe)
	  r += compute(d, cluster, pe);
	/* Produce the result in order. Remove the ordered if it is not
	   needed. */
#pragma omp ordered
	{
	  printf("Cluster %d produced %d for ticket %d\n", cluster, r, t);
	}
      }
    }
  }
}
