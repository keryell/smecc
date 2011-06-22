#include <string>

void bob(int& a)
{
	a+=2;
}

int main()
{
#pragma omp parallel
	{
		int bli=3;
		for (int i=0; i<10; i++)
		{
		#pragma smecy map(PE,i)
			bob(bli);
		}
		return 0;
	}
}
