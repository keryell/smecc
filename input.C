#include <string>

void bob(int* tablo)
{
	for (int i=0; i<10; i++)
		tablo[i]=0;
}

int main()
{
#pragma omp parallel
	{
		int tab[10][100];
		for (int i=0; i<10; i++)
		{
		#pragma smecy map(PE,i) arg(1,out,[10][100],/[i][])
			bob(&tab[i][0]);
		}
		return 0;
	}
}
