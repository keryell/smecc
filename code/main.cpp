//small main for testing purposes

#include "public.hpp"
#include "smecyTranslation.hpp"

int main(int argc, char *argv[]) {
  // Command line processing specific to SME-C:
  bool isSmecy = smecy::processCommandLine(argc, argv);
  //std::string test = CommandlineProcessing::generateStringFromArgList( CommandlineProcessing::generateArgListFromArgcArgv(argc, argv) );
  //std::cout << "command line :" << test << std::endl;

  // Call the ROSE front end, mainly the parser:
  SgProject* project = frontend(argc,argv);

  // Display the list of files to deal with:
  std::vector<SgFile*> files = project->get_fileList();
  for (SgFile * f : files)
    std::cerr << "File " << f->getFileName() << std::endl;
  //if (isSgSourceFile(files[i]))
  //OmpSupport::lower_omp(isSgSourceFile(files[i]));

  // Translating smecy #pragma if the -smecy option has been given
  std::cerr << "Translating smecy" << std::endl;
  if (isSmecy)
    smecy::translateSmecy(project);

  // There is a limitation of ROSE on the AST beautifier that cannot
  // handle more than 1 file:
  if (files.size() > 1)
    std::cerr << "PDF and DOT generation of the AST" << std::endl;
  else {
    generatePDF(*project);
    std::cerr << "Generating DOT" << std::endl;
    generateDOT(*project);
  }

  // If we want to run ROSE unit tests:
  //AstTests::runAllTests(project);

  // Finish by calling the backend: compiler, linker...
  return backend(project);
}


/* A helper function to display ROSE sequence of strings from the debugger */
void display_string_argv(Rose_STL_Container<std::string> fileList) {
  printf("In display_string_argv: %s\n",
         StringUtility::listToString(fileList).c_str());

}
