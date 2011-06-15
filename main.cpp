//small main for testing purposes

#include "smecyParser.tab.hh"
#include "public.h"
#include "smecyAttribute.h"

int main(int argc, char *argv[])
{
	parseSmecyDirective("smecy map(PE)")->print();
	parseSmecyDirective("smecy arg(2,out) arg(3,in,[2][3][4], out)")->print();
	parseSmecyDirective("smecy map(PE,7) arg(19,[113][117],/[2:4][5:9])")->print();
	return 0;
}
