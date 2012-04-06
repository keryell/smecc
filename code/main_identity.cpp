//small main for testing purposes

#include "public.hpp"
#include "smecyTranslation.hpp"

int main(int argc, char *argv[])
{
	SgProject* project=frontend(argc,argv);

	for (auto * f : project->get_fileList())
		std::cerr << "File " << f->getFileName() << std::endl;
		//if (isSgSourceFile(files[i]))
			//OmpSupport::lower_omp(isSgSourceFile(files[i]));

	//generatePDF(*project);
	//std::cerr << "Generating DOT" << std::endl;
	//generateDOT(*project);

	//TEST
	//std::vector<SgFile*> files = project->get_fileList();
	//for (unsigned int i=0; i<files.size(); i++)
		//if (isSgSourceFile(files[i]))
			//OmpSupport::lower_omp(isSgSourceFile(files[i]));

	//AstTests::runAllTests(project);
	return backend(project);
}
