#define N 1000
#define NB_CLUSTERS 4
#define MIN(x, y) ((x) < (y) ? x : y)

#include <stdio.h>

/* Compute a polynomial with Hörner method in a pipelined way with a
   systolic line of PE

   (((ax + b)x + c)x + d)x + e

   This is a spatial developping of the streaming example in
   stream_cluster_PEs.c. So read stream_cluster_PEs.c first for a gentle
   introduction.
*/


/* Values where to compite the polynomial */
int x[] = { 0, 1, -1, 2, 4, 5, 10 };
/* The polynomial is -4x^3 + x^2 + 2x + 3 : */
int poly[] = {0, -4, 1, 2, 3};

/* A stage of a systolic Hörner polynomial computation */
void stage(int *poly, int *x, int *input,
           int *pass_through, int *output) {
  // Compute one Hörner factor y = ax + b
  *output = *input * *x + *poly;
  // Propagate x down the stream:
  *pass_through = *x;
}


void output(int *v, int t, int pe) {
  // In real code there is no such IO available on STHORM...
  printf("Polynomial at t=%d with pe %d = %d\n", t, pe, *v);
}


int main() {
  /* Since the 2 following arrays are read and written at the same time to
     move data through the pipeline, increase the dimension to store
     current t and next t+1 values. Put the time dimension first to limit
     cache line false sharing: */
  int v[2][sizeof(poly)/sizeof(poly[0]) + 1];
  /* To propagate the x unchanged through the pipeline: */
  int path_through[2][sizeof(poly)/sizeof(poly[0])];

  // Launch the thread only once
#pragma omp parallel num_threads(sizeof(poly)/sizeof(poly[0]) + 1)
  /* The time loop.
     It takes len(x)+len(poly) cycles to go through the pipeline */
  for (int t = 0;
       t < sizeof(x)/sizeof(x[0]) + sizeof(poly)/sizeof(poly[0]);
       t++) {
      /* Toggle between 0 and 1 in phase oposition: */
      int current = t & 1;
      int next = ~t & 1;

    /* Spatially spread the computation on the PEs, +1 is for for a PE doing the
       output: */
#pragma omp for schedule(static, 1)
    for (int pe = 0; pe < sizeof(poly)/sizeof(poly[0]) + 1; pe++) {
      if (pe == 0) {
	// Special case for the first stage which taps in the input
	if (t < sizeof(x)/sizeof(x[0])) {
	  v[current][0] = 0;
	  // Only run when there are some data to read
#pragma smecy map(STHORM, 0, 0) arg(1, in) arg(2, in) arg(3, in)	\
                                arg(4, out) arg(5, out)
	  stage(&poly[0], &x[t], &v[current][0],
		&path_through[next][0], &v[next][1]);
	}
      }
      else if (pe == sizeof(poly)/sizeof(poly[0])) {
	/* The PE doing the output is the last stage: it runs only after
	   the data got time to pass through the whole pipeline */
	if (t >= pe)
	  // The output is done on the host, so no #pragma map here
	  output(&v[current][5], t, pe);
      }
      else {
	// The normal computation pipeline stage, with the general schedule
	if (t >= pe &&  t < pe + sizeof(x)/sizeof(x[0])) {
#pragma smecy stage map(STHORM, 0, pe) arg(1, in) arg(2, in) arg(3, in) \
                                       arg(4, out) arg(5, out)
	  stage(&poly[pe], &path_through[current][pe - 1], &v[current][pe],
		&path_through[next][pe], &v[next][pe + 1]);
	}
      }
      // There is an implicit barrier here
    }
  }
  return 0;
}
