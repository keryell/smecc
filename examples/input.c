#include <stdio.h>

void stuff(int thing)
{
	
}

void stuff2(int thing, int other_thing)
{
	
}

int main()
{
	unsigned int tab[10][100];
		
	int b=sizeof(int);
	int c=3;
	#pragma smecy stream_loop
	while (1)
	{
		#pragma smecy stream_node(1) arg(1,out) map(PE,1)
			stuff(b);
		#pragma smecy stream_node(2) arg(1,inout)
			stuff2(b,c);
		#pragma smecy stream_node(2) arg(1,in)
			stuff(b);
	}
	
	return 0;
}
