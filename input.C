#include <string>

void bob(int* tablo, int size)
{
	for (int i=0; i<size; i++)
		tablo[i]=0;
}

int main()
{
#pragma omp parallel
	{
		int* tab = new int[100];
		for (int i=0; i<10; i++)
		{
		#pragma smecy map(PE,i) arg(2,inout) arg(1,out,[10])
			bob(&tab[10*i],10);
		}
		return 0;
	}
}
