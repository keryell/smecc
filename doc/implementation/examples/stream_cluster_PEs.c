#define N 1000
#define NB_CLUSTERS 4
#define NB_PES 16
#define MIN(x, y) ((x) < (y) ? x : y)

#include <stdio.h>

/* Compute a polynomial with Hörner method in a (pipeline) systolic way

   (((ax + b)x + c)x + d)x + e
 */


/* A stage of a systolic Hörner polynomial computation */
void stage(int *poly, int *x, int *input,
           int *pass_through, int *output) {
  // Compute one Hörner factor y = ax + b
  *output = *input * *x + *poly;
  // Propagate x down the stream:
  *pass_through = *x;
}


void output(int *v) {
  // In real code there is no such IO available on STHORM...
  printf("Polynomial = %d\n", *v);
}


int main() {
  /* Values where to compite the polynomial */
  int x[] = { 0, 1, -1, 2, 4, 5, 10 };
  /* The polynomial is -4x^3 + x^2 + 2x + 3 : */
  int poly[] = {0, -4, 1, 2, 3};
  int v[] = {0, 0, 0, 0, 0, 0};
  /* To propagate the x unchanged through the pipeline: */
  int path_through[] = {0, 0, 0, 0, 0};

#pragma smecy stream_loop
  for (int i = 0; i < sizeof(x)/sizeof(x[0]); i++) {
    /* The following could be done with macros */
#pragma smecy map(STHORM, 0, 0) arg(1, in) arg(2, in) arg(3, in) \
                                arg(4, out) arg(5, out)
    stage(&poly[0], &x[i], &v[0], &path_through[0], &v[1]);
#pragma smecy stage map(STHORM, 0, 1) arg(1, in) arg(2, in) arg(3, in) \
                                      arg(4, out) arg(5, out)
    stage(&poly[1], &path_through[0], &v[1], &path_through[1], &v[2]);
#pragma smecy stage map(STHORM, 0, 2) arg(1, in) arg(2, in) arg(3, in) \
                                      arg(4, out) arg(5, out)
    stage(&poly[2], &path_through[1], &v[2], &path_through[2], &v[3]);
#pragma smecy stage map(STHORM, 0, 3) arg(1, in) arg(2, in) arg(3, in) \
                                      arg(4, out) arg(5, out)
    stage(&poly[3], &path_through[2], &v[3], &path_through[3], &v[4]);
#pragma smecy stage map(STHORM, 0, 4) arg(1, in) arg(2, in) arg(3, in) \
                                      arg(4, out) arg(5, out)
    stage(&poly[4], &path_through[3], &v[4], &path_through[4], &v[5]);
#pragma smecy stage map(STHORM, 0, 5) arg(1, in) arg(2, in) arg(3, in) \
                                      arg(4, out) arg(5, out)
    output(&v[5]);
  }
  return 0;
}
