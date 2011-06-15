#ifndef SMECY_AST_CONSTRUCTION_CPP
#define SMECY_AST_CONSTRUCTION_CPP

#include "smecyAstConstruction.h"

namespace smecy
{
	void attachSmecyAttributes(SgProject *sageFilePtr)
	{
		//making a list of all pragma nodes in AST and going through it
		std::vector<SgNode*> allPragmas = NodeQuery::querySubTree(sageFilePtr, V_SgPragmaDeclaration);
		std::vector<SgNode*>::iterator iter;
		for(iter=allPragmas.begin(); iter!=allPragmas.end(); iter++)
		{
			SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(*iter);
			std::string pragmaString = pragmaDeclaration->get_pragma()->get_pragma();
			//std::cout << "Found pragma string : " << pragmaString << std::endl ;
			//parseSmecyDirective(pragmaString)->print();
			std::string pragmaHead;
			std::istringstream stream(pragmaString);
			stream >> pragmaHead;
			if (pragmaHead == "smecy")
			{
				//TODO handle merging with existing smecy attribute
				//TODO handle syntax errors and print nice error message
				pragmaDeclaration->addNewAttribute("smecy", parseSmecyDirective(pragmaString));
				//std::cout << "Adding smecy attribute !" << std::endl;
			}
		}
		return ;
	}
} //namespace smecy

#endif //SMECY_AST_CONSTRUCTION_CPP
