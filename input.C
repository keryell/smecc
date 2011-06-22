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
#pragma smecy map(PE,1) arg(2,inout,[3][4*bli]) arg(1,out)
		bob(bli);
		return 0;
	}
}
