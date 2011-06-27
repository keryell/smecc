int init(int* tablo, int size)
{
	for (int i=0; i<size; i++)
		tablo[i]=0;
	return 0;
}

int main()
{
	int tab[10][100];
	#pragma smecy map(PE,1) arg(1,out,[10][100]) arg(2,inout)
	init(&tab[0][0], 1000);
	return 0;
}
