void bob(int& a)
{
	a+=2;
}

int main()
{
	#pragma omp parallel
	{
		int bli=3;
		bob(bli);
		#pragma smecy arg(0,/[][bob(fete):8], out)
			bob(bli);
	}
	return 0;
}
