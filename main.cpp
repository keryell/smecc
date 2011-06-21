//small main for testing purposes

#include "public.h"
#include "smecyAttribute.h"
#include "smecyAstConstruction.h"

int main(int argc, char *argv[])
{
	SgProject* project=frontend(argc,argv);
	
	smecy::attachAttributes(project);
	//smecy::extractExpressions(project);
	smecy::parseExpressions(project);
	
	generatePDF(*project);
	generateDOT(*project);
	
	AstTests::runAllTests(project);
	return backend(project);
}
