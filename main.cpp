//small main for testing purposes

#include "public.h"
#include "smecyAttribute.h"
#include "smecyAstConstruction.h"

int main(int argc, char *argv[])
{
	SgProject* project=frontend(argc,argv);
	
	smecy::attachSmecyAttributes(project);
	
	AstTests::runAllTests(project);
	return backend(project);
}
