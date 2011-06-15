void bob(int& a)
{
	a+=2;
}

int main()
{
	int bli=3;
	#pragma smecy map(PE,5) arg(0,inout)
		bob(bli);
	#pragma smecy map(PE,5) arg(0,in)
		bob(bli);
	return 0;
}
