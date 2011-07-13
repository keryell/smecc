void stuff(int thing, int other_thing)
{
	
}

void stuff2(int thing, int other_thing)
{
	
}

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
			stuff2(b,c);
	}
	
	return 0;
}
