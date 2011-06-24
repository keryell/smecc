void init(int* tablo, int size)
{
	for (int i=0; i<10; i++)
		tablo[i]=0;
}

int main()
{
	int tab[10][100];
#pragma omp parallel for
	for (int i=0; i<10; i++)
	{
#pragma smecy map(PE,i) arg(1,out,[10][100],/[i][]) arg(2,in)
		init(&tab[i][0], 100);
	}
	return 0;
}
