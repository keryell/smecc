int init(unsigned int* tablo, int size)
{
	for (int i=0; i<size; i++)
		tablo[i]=0;
	return 0;
}

int main()
{
	unsigned int tab[10][100];
	for (int i=0; i<10; i++)
	{
		int b;
		#pragma smecy map(PE,1) arg(1,out,/[1][]) if(i)
			int a = init(&tab[i][0], 100);
	}
	return 0;
}
