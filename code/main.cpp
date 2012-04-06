//small main for testing purposes

#include "public.hpp"
#include "smecyTranslation.hpp"

int main(int argc, char *argv[])
{
	//command line processing
	bool isSmecy = smecy::processCommandLine(argc, argv);
	//std::string test = CommandlineProcessing::generateStringFromArgList( CommandlineProcessing::generateArgListFromArgcArgv(argc, argv) );
	//std::cout << "command line :" << test << std::endl;

	SgProject* project=frontend(argc,argv);

	//TEST
	std::vector<SgFile*> files = project->get_fileList();
	for (SgFile * f : files)
		std::cerr << "File " << f->getFileName() << std::endl;
		//if (isSgSourceFile(files[i]))
			//OmpSupport::lower_omp(isSgSourceFile(files[i]));

	//translating smecy
	std::cerr << "Translating smecy" << std::endl;
	if (isSmecy)
		smecy::translateSmecy(project);

	generatePDF(*project);
	std::cerr << "Generating DOT" << std::endl;
	generateDOT(*project);

	//AstTests::runAllTests(project);
	return backend(project);
}
