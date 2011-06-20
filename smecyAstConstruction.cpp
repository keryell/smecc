#ifndef SMECY_AST_CONSTRUCTION_CPP
#define SMECY_AST_CONSTRUCTION_CPP

#include "smecyAstConstruction.h"

//===================================================================
// Implements functions used during the translation of SMECY programs
//===================================================================

namespace smecy
{
	void attachAttributes(SgProject *sageFilePtr)
	{
		//making a list of all pragma nodes in AST and going through it
		std::vector<SgNode*> allPragmas = NodeQuery::querySubTree(sageFilePtr, V_SgPragmaDeclaration);
		std::vector<SgNode*>::iterator iter;
		for(iter=allPragmas.begin(); iter!=allPragmas.end(); iter++)
		{
			SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(*iter);
			std::string pragmaString = pragmaDeclaration->get_pragma()->get_pragma();
			std::string pragmaHead;
			std::istringstream stream(pragmaString);
			stream >> pragmaHead;
			if (pragmaHead == "smecy")
			{
				std::cout << "Found pragma string : " << pragmaString << std::endl ;
				smecy::parseDirective(pragmaString)->print();
				//TODO handle merging with existing smecy attribute
				//TODO handle syntax errors and print nice error message
				pragmaDeclaration->addNewAttribute("smecy", smecy::parseDirective(pragmaString));
				//std::cout << "Adding smecy attribute !" << std::endl;
			}
		}
		return ;
	}
	
	void expressionExtractor::visit(SgNode* astNode)
	{
		SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(astNode);
		if (pragmaDeclaration != NULL)
		{
			std::string pragmaString = pragmaDeclaration->get_pragma()->get_pragma();
			std::string pragmaHead;
			std::istringstream stream(pragmaString);
			stream >> pragmaHead;
			if (pragmaHead == "smecy")
			{
				//we get the list of all expressions contained in the smecy directive
				smecy::Attribute* attribute = (smecy::Attribute*)pragmaDeclaration->getAttribute("smecy");
				std::vector<std::string> exprList = attribute->getExpressionList();
				
				//we build a declaration for each of them
				for (unsigned int i=0; i<exprList.size(); i++)
				{
					std::cout << "YOUPIIIII" <<std::endl;
					std::ostringstream declaration("");
					declaration << std::endl << "int smecy" << i << " = " << exprList[i] << ";" ;
					SageInterface::addTextForUnparser(pragmaDeclaration,declaration.str(),AstUnparseAttribute::e_before);
				}
			}
		}
	}
	
	void extractExpressions(SgProject *sageFilePtr)
	{
		expressionExtractor extractor;
		extractor.traverseInputFiles(sageFilePtr, preorder);
	}
} //namespace smecy

#endif //SMECY_AST_CONSTRUCTION_CPP
