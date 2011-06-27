//small main for testing purposes

#include "public.h"
#include "smecyTranslation.h"

int main(int argc, char *argv[])
{
	SgProject* project=frontend(argc,argv);
	
	smecy::translateSmecy(project);
	
	//generatePDF(*project);
	generateDOT(*project);
	
	//AstTests::runAllTests(project);
	return backend(project);
}
