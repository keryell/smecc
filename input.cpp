void bob(int& a)
{
	a+=2;
}

int main()
{
	#pragma omp parallel
	{
		int bli=3;
		#pragma smecy map(PE,func(bob,17)azea) arg(0,inout)
			bob(bli);
		#pragma smecy map(PE,5) arg(0,in,/[f(2):2][3:CONSTANT])
			bob(bli);
	}
	return 0;
}
