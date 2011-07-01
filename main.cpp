//small main for testing purposes

#include "public.h"
#include "smecyTranslation.h"

int main(int argc, char *argv[])
{
	SgProject* project=frontend(argc,argv);
	
	//translating smecy
	std::cout << "Translating smecy" << std::endl;
	smecy::translateSmecy(project);
	
	//generatePDF(*project);
	std::cout << "Generating DOT" << std::endl;
	generateDOT(*project);
	
	//TEST
	std::vector<SgFile*> files = project->get_fileList();
	for (unsigned int i=0; i<files.size(); i++)
		if (isSgSourceFile(files[i]))
			OmpSupport::lower_omp(isSgSourceFile(files[i]));
	
	//AstTests::runAllTests(project);
	return backend(project);
}
