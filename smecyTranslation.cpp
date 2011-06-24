#ifndef SMECY_TRANSLATION_CPP
#define SMECY_TRANSLATION_CPP

#include "smecyTranslation.h"

//===================================================================
// Implements functions used during the translation of SMECY programs
//===================================================================

namespace smecy
{
	/* First steps :
	attaching attributes and parsing expressions from pragmas.
	TODO : merge so as to go through the pragmas only once ?
	*/
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
					
					//...store it in the attribute...
					attribute->addParsedExpression(expr);
					
					//...and remove the declaration
					SageInterface::removeStatement(decl);
					//TODO remove the rest of the declaration from memory
				}
			}
		}
		return ;
	}
	
	/* AddSmecyXXX functions :
	these functions add calls to smecy API to the AST
	TODO : a little refactoring since they all look alike
	*/
	void addSmecyInclude(SgProject *sageFilePtr)
	{
		//FIXME compatible with multi-file projects ?
		//TODO check if the include is already present
		SgScopeStatement* scope = SageInterface::getFirstGlobalScope(sageFilePtr);
		SageInterface::insertHeader("smecy.h", PreprocessingInfo::after, false, scope);
	}
	
	void addSmecySet(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap)
	{
		//building parameters to build the func call (bottom-up building)
		SgExprListExp * exprList = SageBuilder::buildExprListExp(mapName, mapNumber, functionToMap);
		SgName name("SMECY_set");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall);
	}
	
	void addSmecyLaunch(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap)
	{
		//building parameters to build the func call (bottom-up building)
		SgExprListExp * exprList = SageBuilder::buildExprListExp(mapName, mapNumber, functionToMap);
		SgName name("SMECY_launch");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall);
	}
	
	void addSmecySendArg(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value)
	{
		//building parameters to build the func call (bottom-up building)
		SgExpression* argNumberExpr = SageBuilder::buildIntVal(argNumber);
		SgExprListExp * exprList = SageBuilder::buildExprListExp(mapName, mapNumber, functionToMap, argNumberExpr, typeDescriptor, value);
		SgName name("SMECY_send_arg");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall);
	}
	
	/*void processSendArgs(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap, Attribute* attribute)
	{
		std::vector<Arg> argList = attribute->getArgList();
		for (unsigned int i=0; i<argList.size(); i++)
		{
			if ((argList[i].argType==_arg_in or argList[i].argType==_arg_inout) and (argList[i].argSize.size()==0))
			{
				int argNumber = argList[i].argNumber-1;	//counting from 0 instead of from 1
				SgExpression* typeDescriptor = getArgTypeDescriptor(SageInterface::getNextStatement(target), argNumber);
				SgExpression* value = getArgRef(SageInterface::getNextStatement(target), argNumber);
				addSmecySendArg(target, mapName, mapNumber, functionToMap, argNumber, typeDescriptor, value);
			}
		}
	}*/
	
	void addSmecyGetArg(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value)
	{
		//building parameters to build the func call (bottom-up building)
		SgExpression* argNumberExpr = SageBuilder::buildIntVal(argNumber);
		SgExprListExp * exprList = SageBuilder::buildExprListExp(mapName, mapNumber, functionToMap, argNumberExpr, typeDescriptor, value);
		SgName name("SMECY_get_arg");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall);
	}
	
	/*void processGetArgs(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap, Attribute* attribute)
	{
		std::vector<Arg> argList = attribute->getArgList();
		for (unsigned int i=0; i<argList.size(); i++)
		{
			if ((argList[i].argType==_arg_out or argList[i].argType==_arg_inout) and (argList[i].argSize.size()==0))
			{
				int argNumber = argList[i].argNumber-1;	//counting from 0 instead of 1
				SgExpression* typeDescriptor = getArgTypeDescriptor(SageInterface::getNextStatement(target), argNumber);
				SgExpression* value = getArgRef(SageInterface::getNextStatement(target), argNumber);
				addSmecyGetArg(target, mapName, mapNumber, functionToMap, argNumber, typeDescriptor, value);
			}
		}
	}*/
	
	
	/* SgXXX extractors :
	functions to extract specific informations from the AST
	TODO : improve error messages
	*/
	SgExpression* getFunctionRef(SgStatement* functionCall)
	{
		//temp variables for downcasting
		SgExpression* tempExp;
		SgNode* tempNode;
	
		//checking the SgStatement in parameter and extracting func name TODO add !=NULL checking
		SgExprStatement* exprSmt = isSgExprStatement(functionCall);
		tempExp = exprSmt->get_expression();
		SgFunctionCallExp* functionCallExp = isSgFunctionCallExp(tempExp);
		tempExp = functionCallExp->get_function();
		tempNode = SageInterface::deepCopyNode(tempExp);
		return isSgExpression(tempNode);
	}
	
	SgExprListExp* getArgList(SgStatement* functionCall)
	{
		//temp variables for downcasting
		SgExpression* tempExp;
	
		//checking the SgStatement in parameter and extracting func name TODO add !=NULL checking
		SgExprStatement* exprSmt = isSgExprStatement(functionCall);
		tempExp = exprSmt->get_expression();
		SgFunctionCallExp* functionCallExp = isSgFunctionCallExp(tempExp);
		return functionCallExp->get_args();
	}
	
	SgExpression* getArgRef(SgStatement* functionCall, int argNumber)
	{	
		SgExprListExp* args = getArgList(functionCall);
		if ((int)args->get_expressions().size()>=argNumber)
			return args->get_expressions()[argNumber];
		else
		{
			std::cerr << "Invalid arg number for extraction." << std::endl;
			throw 0;
		}
	}
	
	SgExpression* getArgTypeDescriptor(SgStatement* functionCall, int argNumber)
	{
		SgExpression* argRef = getArgRef(functionCall, argNumber);
		SgType* argType = argRef->get_type();
		SgScopeStatement* scope = SageInterface::getScope(functionCall);
		
		//TODO check if it is possible to extract a string from a type
		if (isSgTypeInt(argType))
			return SageBuilder::buildOpaqueVarRefExp("int", scope);
		else
		{
			std::cerr << "Unsupported type." << std::endl;
			throw 0;
		}
	}
	
	SgExpression* getArgVectorTypeDescriptor(SgStatement* functionCall, int argNumber)
	{
		SgExpression* argRef = getArgRef(functionCall, argNumber);
		SgType* argType = argRef->get_type();
		if (!SageInterface::isPointerType(argType))
		{
			std::cerr << "Argument was not a pointer." << std::endl;
			throw 0;
		}
		SgType* elementType = SageInterface::getElementType(argType);	//FIXME what will it to with int** for example ?
		SgScopeStatement* scope = SageInterface::getScope(functionCall);
		
		if (isSgTypeInt(elementType))
			return SageBuilder::buildOpaqueVarRefExp("int", scope);
		else
		{
			std::cerr << "Unsupported type." << std::endl;
			throw 0;
		}
	}
	
	SgExpression* copy(SgExpression* param)
	{
		SgNode* temp = SageInterface::deepCopyNode(param);
		return isSgExpression(temp);
	}
	
	/* High-level functions for arguments :
	Checks the correct dimension and layout in memory of all arguments
	then, creates the corresponding API calls
	*/
	void processArgs(SgStatement* target, Attribute* attribute)
	{
		//we go through the function's parameters
		SgStatement* funcToMap = SageInterface::getNextStatement(target);
		SgExprListExp* argList = getArgList(funcToMap);
		for (unsigned int i=0; i<argList->get_expressions().size(); i++)
		{
			
		}
	}
	
	/* Top-level function :
	this is the function that should be called to translate smecy pragmas
	into calls to the SMECY API
	TODO : finish it !
	*/
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
				//parameters
				smecy::Attribute* attribute = (smecy::Attribute*)pragmaDeclaration->getAttribute("smecy");
				SgExpression* mapName = attribute->getMapName();
				SgExpression* mapNumber = attribute->getMapNumber();
				SgExpression* funcToMap = getFunctionRef(SageInterface::getNextStatement(pragmaDeclaration));

				addSmecySet(pragmaDeclaration, mapName, mapNumber, funcToMap);
				//processSendArgs(pragmaDeclaration, mapName, mapNumber, funcToMap, attribute);
				addSmecyLaunch(pragmaDeclaration, copy(mapName), copy(mapNumber), copy(funcToMap));
				//processGetArgs(pragmaDeclaration, mapName, mapNumber, funcToMap, attribute);
			}
		}
	}
} //namespace smecy

#endif //SMECY_TRANSLATION_CPP
