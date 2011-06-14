//small main for testing purposes

#include "smecyParser.tab.hh"
#include "public.h"

int main(int argc, char *argv[])
{
	parseSmecyDirective("smecy map(PE)");
	parseSmecyDirective("smecy arg(2,out) arg(3,in)");
	parseSmecyDirective("smecy map(PE,7) arg(2,out)");
	return 0;
}
