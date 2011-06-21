void bob(int& a)
{
	a+=2;
}

int main()
{

	int bli=3;
	#pragma smecy map(PE,bli)
	{
		bob(bli);
	}
	return 0;
}
