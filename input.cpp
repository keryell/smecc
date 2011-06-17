void bob(int& a)
{
	a+=2;
}

int main()
{
	#pragma omp parallel
	{
		int bli=3;
		#pragma smecy map(PE,bli)
			bob(bli);
		#pragma smecy arg(7,[100][bli],/[][bob(bli):8][bli], out)
			bob(bli);
	}
	return 0;
}
