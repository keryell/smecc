int init(unsigned int* tablo, int size)
{
	for (int i=0; i<size; i++)
		tablo[i]=0;
	return 0;
}

void stuff()
{
	
}

int main()
{
	unsigned int tab[10][100];
		
	#pragma smecy stream_loop
	while (1)
	{
		#pragma smecy stream_node(1)
			stuff();
		#pragma smecy stream_node(2)
			stuff();
	}
	
	return 0;
}
