int init(unsigned int* tablo, int size)
{
	for (int i=0; i<size; i++)
		tablo[i]=0;
	return 0;
}

int main()
{
	unsigned int tab[10][100];
	#pragma omp sections
	{
		#pragma omp section
			#pragma smecy map(PE,0) arg(1,out,/[0:4][])
				init(&tab[0][0], 500);
		#pragma omp section
			#pragma smecy map(PE,1) arg(1,out,/[5:9][])
				init(&tab[5][0], 500);
	}
	return 0;
}
