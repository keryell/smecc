//small main for testing purposes

#include "smecyParser.tab.hh"
#include "public.h"

int main(int argc, char *argv[])
{
	parseSmecyDirective("input");
	parseSmecyDirective("input");
	parseSmecyDirective("input");
	return 0;
}
