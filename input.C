int init(unsigned int* tablo, int size)
{
	for (int i=0; i<size; i++)
		tablo[i]=0;
	return 0;
}

void stuff(int thing, int other_thing)
{
	
}

void stuff2(int thing)
{
	
}

typedef struct test
{
	int a;
	int b[12];
	int c[5][8];
};

int main()
{
	unsigned int tab[10][100];
		
	int b=2;
	int c=3;
	#pragma smecy stream_loop
	while (1)
	{
		#pragma smecy stream_node(1)
			stuff(b,c);
		#pragma smecy stream_node(2)
			stuff2(b);
	}
	
	return 0;
}
