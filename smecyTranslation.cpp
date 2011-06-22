#ifndef SMECY_TRANSLATION_CPP
#define SMECY_TRANSLATION_CPP

#include "smecyTranslation.h"

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
	
	void extractExpressions(SgProject *sageFilePtr)
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
				//we get the list of all expressions contained in the smecy directive
				smecy::Attribute* attribute = (smecy::Attribute*)pragmaDeclaration->getAttribute("smecy");
				std::vector<std::string> exprList = attribute->getExpressionList();
				
				//we build a declaration for each of them
				for (unsigned int i=0; i<exprList.size(); i++)
				{
					std::ostringstream declaration("");
					declaration << std::endl << "int smecy" << i << " = " << exprList[i] << ";" ;
					SageInterface::addTextForUnparser(pragmaDeclaration,declaration.str(),AstUnparseAttribute::e_before);
				}
			}
		}
		return ;
	}
	
	void parseExpressions(SgProject *sageFilePtr)
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
				//we get the list of all expressions contained in the smecy directive
				smecy::Attribute* attribute = (smecy::Attribute*)pragmaDeclaration->getAttribute("smecy");
				std::vector<std::string> exprList = attribute->getExpressionList();
				
				// Insert new code into the scope represented by the statement (applies to SgScopeStatements)
				MiddleLevelRewrite::ScopeIdentifierEnum scope = MidLevelCollectionTypedefs::SurroundingScope;
				SgStatement* target = pragmaDeclaration;//SageInterface::getScope(pragmaDeclaration);
				
				//we build a declaration for each of them
				for (unsigned int i=0; i<exprList.size(); i++)
				{
					std::ostringstream declaration("");
					declaration << std::endl << "int smecy" << i << " = " << exprList[i] << ";" ;
					MiddleLevelRewrite::insert(target,declaration.str(),scope,
						MidLevelCollectionTypedefs::BeforeCurrentPosition);
						
					//now, we can collect the expression in the variable declaration...
					SgVariableDeclaration* decl = isSgVariableDeclaration(SageInterface::getPreviousStatement(pragmaDeclaration));
					if (!decl)
						std::cerr << "Found invalid variable declaration while parsing expressions." << std::endl;
					SgInitializedName* initName = SageInterface::getFirstInitializedName(decl);
					SgAssignInitializer* initializer = isSgAssignInitializer(initName->get_initializer());
					if (!initializer)
						std::cerr << "Found invalid initializer while parsing expressions." << std::endl;
					SgExpression* expr = initializer->get_operand();
					//TODO now that we have the expr, put it somewhere
					
					//...and remove the declaration
					SageInterface::removeStatement(decl);
					//TODO remove the rest of the declaration from memory
				}
			}
		}
		return ;
	}
	
	void addSmecyInclude(SgProject *sageFilePtr)
	{
		//FIXME compatible with multi-file projects ?
		SgScopeStatement* scope = SageInterface::getFirstGlobalScope(sageFilePtr);
		SageInterface::insertHeader("smecy.h", PreprocessingInfo::after, false, scope);
	}
	
	void addSmecySet(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber , SgStatement* functionToMap )
	{
		//temp variables for downcasting
		SgExpression* tempExp;
		SgNode* tempNode;
	
		//checking the SgStatement in parameter and extracting func name TODO add !=NULL checking
		SgExprStatement* funcToMap = isSgExprStatement(functionToMap);
		tempExp = funcToMap->get_expression();
		SgFunctionCallExp* funcToMapFuncExp = isSgFunctionCallExp(tempExp);
		tempExp = funcToMapFuncExp->get_function();
		tempNode = SageInterface::deepCopyNode(tempExp);
		SgExpression* funcToMapRef = isSgExpression(tempNode);
		
		//TODO add pe and instance args
	
		//building parameters to build the func call (bottom-up building)
		SgExprListExp * exprList = SageBuilder::buildExprListExp(mapName, mapNumber, funcToMapRef);
		SgName name("SMECY_set");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall);
	}
	
	void translateSmecy(SgProject *sageFilePtr)
	{
		//adding #include "smecy.h"
		addSmecyInclude(sageFilePtr);
		
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
				//the attribute is where we get information
				smecy::Attribute* attribute = (smecy::Attribute*)pragmaDeclaration->getAttribute("smecy");
			
				// TEST
				SgExpression* testMapName = SageBuilder::buildStringVal("test");
				SgExpression* testMapNumber = SageBuilder::buildIntVal(3);
				// /TEST
				addSmecySet(pragmaDeclaration, testMapName, testMapNumber ,SageInterface::getNextStatement(pragmaDeclaration));
			}
		}
	}
} //namespace smecy

#endif //SMECY_TRANSLATION_CPP
