//small main for testing purposes

#include "public.h"
#include "smecyTranslation.h"

int main(int argc, char *argv[])
{
	//command line processing
	bool isSmecy = smecy::processCommandLine(argc, argv);
	//std::string test = CommandlineProcessing::generateStringFromArgList( CommandlineProcessing::generateArgListFromArgcArgv(argc, argv) );
	//std::cout << "command line :" << test << std::endl;

	SgProject* project=frontend(argc,argv);
	
	//translating smecy
	std::cerr << "Translating smecy" << std::endl;
	if (isSmecy)
		smecy::translateSmecy(project);
	
	//generatePDF(*project);
	std::cerr << "Generating DOT" << std::endl;
	generateDOT(*project);
	
	//TEST
	//std::vector<SgFile*> files = project->get_fileList();
	//for (unsigned int i=0; i<files.size(); i++)
		//if (isSgSourceFile(files[i]))
			//OmpSupport::lower_omp(isSgSourceFile(files[i]));
	
	//AstTests::runAllTests(project);
	return backend(project);
}
