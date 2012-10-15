void functionA() {
}

void functionB() {
}


int main() {
  //#pragma omp parallel sections
#pragma omp parallel
#pragma omp sections
  {
#pragma omp section
    {
      functionA();
    }
#pragma omp section
    {
      functionB();
    }
  }

}
