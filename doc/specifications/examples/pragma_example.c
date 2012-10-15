/* To compile this program on Linux, try:

   make CFLAGS='-std=c99 -Wall' pragma_example

   To run:
   ./pragma_example; echo $?
   It should print 0 if OK.

   You can even compile it to run on multicore SMP for free with

   make CFLAGS='-std=c99 -fopenmp -Wall' pragma_example

   To verify there are really some clone() system calls that create the threads:
   strace -f ./pragma_example ; echo $?

   You can notice that the #pragma smecy are ignored (the project is
   on-going :-) ) but that the program produces already correct results in
   sequential execution and parallel OpenMP execution.

   Enjoy!

   Remi.Barrere@thalesgroup.com
   Ronan.Keryell@hpc-project.com
   for ARTEMIS SMECY European project.
*/

#include <stdbool.h>

/* function Gen

   Example of old C89 array use-case where the size is unknown. Note that
   this implies some nasty access linearization with array with more than
   1 dimension.
 */
void Gen(int *out, int size) {
  // Can be executed in parallel
#pragma omp parallel for
  for (int i = 0; i < size; i++)
    out [i] = 0;
}


/* function Add

   Nice C99 array with dynamic size definition. Note this implies having
   array size given first
*/
void Add(int size, int in[size], int out[size]) {
  // Can be executed in parallel
#pragma omp parallel for
  for (int i = 0; i < size; i++)
    out [i] = in [i] + 1;
}


/* function Test */
bool Test(int size, int in[size]) {
  bool ok = true;
  /* Can be executed in parallel, ok is initialized from global value and
     at loop exit ok is the && operation between all the local ok
     instances: */
#pragma omp parallel for reduction(&&:ok)
  for (int i = 0; i < size; i++)
    /* We cannot have this simple code here:
       if (in [i] != 2)
         exit(-1) ;
       because a loop or a fonction with exit() cannot be executed in parallel.

       Proof: there is a parallel execution interleaving that may execute
       some computations in some threads with a greater i that the one
       executing the exit() done on another thread. So the causality is
       not respected.

       Anyway, in an heterogenous execution, just think about how to
       implement the exit() operating system call from an
       accelerator... No hope. :-)

       So use a reduction instead and return the status for later
       inspection:
    */
    ok &= (in[i] == 2);

  // Return false if at least one in[i] is not equal to 2:
  return ok;
}


/* main */
int main(int argc, char* argv[]) {
  int tab[6][200];
  // Gen is mapped on GPP 0, it produced (out) an array written to arg 1:
#pragma smecy map(GPP, 0) arg(1, [6][200], out)
  /* Note there is an array linearization here, since we give a 2D array
     to Gen() that uses it . This is bad programming style, but it is just
     to show it can be handled in the model :-) */
  Gen((int *) tab, 200*6);

  // Launch different things in parallel:
#pragma omp parallel sections
  {
    // Do one thing in parallel...
#pragma omp section
    {
      /* Map this "Add" call to PE 0, arg 2 is communicated as input as an
	 array of "int [3][200]", and after execution arg 3 is
	 communicated out as an array of "int [3][200]"

	 Note the aliasing of the 2 last arguments. Just to show we can
	 handle it. :*/
#pragma smecy map(PE, 0) arg(2, [3][200], in) arg(3, [3][200], out)
      Add(200*3, (int *) tab, (int *) tab);
    }
    // ...with another thing
#pragma omp section
    {
      /* Map this "Add" call to PE 1, arg 2 is communicated as input as an
	 array of "int [3][200]" from address tab[3][0], that is the
	 second half of tab, and after execution arg 3 is communicated out
	 as an array of "int [3][200]", that is the second half of tab

	 Note the aliasing of the 2 last arguments. Just to show we can
	 handle it. :*/
#pragma smecy map(PE, 1) arg(2, [3][200], in) \
                         arg(3, [3][200], out)
      Add(200*3, &tab[3][0], &tab[3][0]);
    }
  }

  // Launch different things in parallel:
#pragma omp parallel sections
  {
#pragma omp section
    {
#pragma smecy map(PE, 2) arg(2, [2][200], in) arg(3, [2][200], out)
      Add(200*2, (int *) tab, (int *) tab);
    }
#pragma omp section
    {
#pragma smecy map(PE, 3) arg(2, [2][200], in) arg(3, [2][200], out)
      Add(200*2, &tab[2][0], &tab[2][0]);
    }
#pragma omp section
    {
#pragma smecy map(PE, 4) arg(2, [2][200], in) arg(3, [2][200], out)
      Add(200*2, &tab[4][0], &tab[4][0]);
    }
  }
  // An example where arg 2 is just used as a whole implicitly:
#pragma smecy map(GPP, 0) arg(2, in)
  bool result = Test(200*6, (int *) tab);
  // Return non 0 if the computation went wrong:
  return !result;
}
