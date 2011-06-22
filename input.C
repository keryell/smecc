void bob(int& a)
{
	a+=2;
}

int main()
{

	int bli=3;
#pragma smecy map(PE,1) arg(2,inout,[3][4*bli])
	bob(bli);
	return 0;
}
