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
				std::string test = "int lapin=42;";
				SageInterface::addTextForUnparser(pragmaDeclaration,test,AstUnparseAttribute::e_before);
			
				/*// It is up to the user to link the implementations of these functions link time
				std::string codeAtTopOfBlock    = "void myTimerFunctionStart(); myTimerFunctionStart();";
				std::string codeAtBottomOfBlock = "void myTimerFunctionEnd(); myTimerFunctionEnd();";

				// Insert new code into the scope represented by the statement (applies to SgScopeStatements)
				MiddleLevelRewrite::ScopeIdentifierEnum scope = MidLevelCollectionTypedefs::StatementScope;

				// Insert the new code at the top and bottom of the scope represented by block
				SgStatement* target = isSgBasicBlock(pragmaDeclaration->get_parent());
				if (target!=NULL)
				{
					//FIXME not stable
					//MiddleLevelRewrite::insert(target,codeAtTopOfBlock,scope,MidLevelCollectionTypedefs::TopOfCurrentScope);
					//MiddleLevelRewrite::insert(target,codeAtBottomOfBlock,scope,MidLevelCollectionTypedefs::BottomOfCurrentScope);
				}
				else
				std::cout << "PAS DE PAREEENT" << std::endl;*/
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
