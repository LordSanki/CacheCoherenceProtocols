#include <stdio.h>
#include <math.h>

#define MATRIX_DIM 1000
#define NUM_ITER 10 
/* Try the problems with higher value of MATRIX_DIM and NUM_ITER */

double A[MATRIX_DIM][MATRIX_DIM], 
  B[MATRIX_DIM][MATRIX_DIM], 
  Y[MATRIX_DIM][MATRIX_DIM],
  sum,
  amax;

int n, i, j, k;

double abs_max(double x, double y, double z)
{
  double u, v, w; 
  u = fabs(x);
  v = fabs(y);
  w = fabs(z);
  if (u<v) {
    if (v<w)
      return w;
    else 
      return v;
  }
  else {
    if (u<w)
      return w;
    else
      return u;
  }
}

int main() 
{
  /* Initialize Matrices A and B */
  for (i=0; i<MATRIX_DIM; i++) {
    for (j=0; j<MATRIX_DIM; j++) {
      A[i][j] = i / (double) MATRIX_DIM;
      B[i][j] = i*i / (double) (MATRIX_DIM * MATRIX_DIM);
      Y[i][j] = i*5 / (double) MATRIX_DIM;
    }
  }

  /* Find the sum of elements of Y */
  sum = 0;
  for (i=0; i<MATRIX_DIM; i++) {
    for (j=0; j<MATRIX_DIM; j++) {
      sum = sum + Y[i][j];
    }
  }
  printf("Sum = %f\n", sum);

  /* Find absolute maximum of elements in matrix Y */
  amax = 0;
  for (i=0; i<MATRIX_DIM; i++) {
    for (j=0; j<MATRIX_DIM; j++) {
      amax = abs_max(amax, A[i][j], B[i][j]);
    }
  }

  printf("- sum of matrix = %f\n", sum);
  printf("- absmax of matrix = %f\n", amax);
}
