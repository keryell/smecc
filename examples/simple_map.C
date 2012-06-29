// This #include do not get through ROSE... :-(
#include <iostream>

#define N 10
#define M 5

void init(int* array, int size) {
  for (int i = 0; i < size; i++)
    array[i] = i;
}

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
    std::cout << "Line " << i << " :";
    for (int j = 0; j < M; j++)
      std::cout << " " << tab[i][j];
    std::cout << std::endl;
  }
  return 0;
}
