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
						std::cerr << "Error: Found invalid variable declaration while parsing expressions." << std::endl;
					SgInitializedName* initName = SageInterface::getFirstInitializedName(decl);
					SgAssignInitializer* initializer = isSgAssignInitializer(initName->get_initializer());
					if (!initializer)
						std::cerr << "Error: Found invalid initializer while parsing expressions." << std::endl;
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
		SageInterface::insertHeader("smecy.h", PreprocessingInfo::after, true, scope);
	}
	
	void addSmecySet(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap)
	{
		//building parameters to build the func call (bottom-up building)
		SgExprListExp * exprList = SageBuilder::buildExprListExp(copy(mapName), copy(mapNumber), copy(functionToMap));
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
		SgExprListExp * exprList = SageBuilder::buildExprListExp(copy(mapName), copy(mapNumber), copy(functionToMap));
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
		SgExprListExp * exprList = SageBuilder::buildExprListExp(copy(mapName), copy(mapNumber), copy(functionToMap), copy(argNumberExpr),
				copy(typeDescriptor), copy(value));
		SgName name("SMECY_send_arg");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall);
	}
	
	void addSmecySendArgVector(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value, SgExpression* size)
	{
		//building parameters to build the func call (bottom-up building)
		SgExpression* argNumberExpr = SageBuilder::buildIntVal(argNumber);
		SgExprListExp * exprList = SageBuilder::buildExprListExp(copy(mapName), copy(mapNumber), copy(functionToMap), copy(argNumberExpr),
				copy(typeDescriptor), copy(value), copy(size));
		SgName name("SMECY_send_arg_vector");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall);
	}
	
	void addSmecyGetArgVector(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value, SgExpression* size)
	{
		//building parameters to build the func call (bottom-up building)
		SgExpression* argNumberExpr = SageBuilder::buildIntVal(argNumber);
		SgExprListExp * exprList = SageBuilder::buildExprListExp(copy(mapName), copy(mapNumber), copy(functionToMap), copy(argNumberExpr),
				copy(typeDescriptor), copy(value), copy(size));
		SgName name("SMECY_get_arg_vector");
		SgType* type = SageBuilder::buildVoidType();
		SgScopeStatement* scope = SageInterface::getScope(target);
		
		//building the function call
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, type, exprList, scope);
		SageInterface::insertStatement(target, funcCall, false);
	}
	
	SgExpression* smecyReturn(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap, SgType* returnType)
	{
		//parameters for the builder
		SgScopeStatement* scope = SageInterface::getScope(target);
		SgExpression* typeDescriptor = SageBuilder::buildOpaqueVarRefExp(returnType->unparseToString(), scope);
		SgExprListExp * exprList = SageBuilder::buildExprListExp(copy(mapName), copy(mapNumber), copy(functionToMap), copy(typeDescriptor));
		SgName name("SMECY_get_return");

		//since there's no proper builder for functionCallExp, we will extract it from a functionCallStmt		
		SgExprStatement* funcCall = SageBuilder::buildFunctionCallStmt(name, returnType, exprList, scope);
		return funcCall->get_expression();
	}
	
	/* SgXXX extractors :
	functions to extract specific informations from the AST
	TODO : improve error messages
	*/
	SgExpression* getFunctionRef(SgStatement* functionCall)
	{
		//temp variables for downcasting
		SgExpression* tempExp;
		SgNode* tempNode;

		//going down the AST
		SgFunctionCallExp* functionCallExp = getFunctionCallExp(functionCall);
		tempExp = functionCallExp->get_function();
		tempNode = SageInterface::deepCopyNode(tempExp);
		return isSgExpression(tempNode);
	}
	
	SgExprListExp* getArgList(SgStatement* functionCall)
	{
		SgFunctionCallExp* functionCallExp = getFunctionCallExp(functionCall);
		return functionCallExp->get_args();
	}
	
	SgExpression* getArgRef(SgStatement* functionCall, int argNumber)
	{	
		SgExprListExp* args = getArgList(functionCall);
		if ((int)args->get_expressions().size()>=argNumber)
			return args->get_expressions()[argNumber];
		else
		{
			std::cerr << "Error: Invalid arg number for extraction." << std::endl;
			throw 0;
		}
	}
	
	SgExpression* getArgTypeDescriptor(SgStatement* functionCall, int argNumber)
	{
		SgExpression* argRef = getArgRef(functionCall, argNumber);
		SgType* argType = argRef->get_type();
		SgScopeStatement* scope = SageInterface::getScope(functionCall);
		
		return SageBuilder::buildOpaqueVarRefExp(argType->unparseToString(), scope);
	}
	
	SgExpression* getArgVectorTypeDescriptor(SgStatement* functionCall, int argNumber)
	{
		SgExpression* argRef = getArgRef(functionCall, argNumber);
		SgType* argType = argRef->get_type();
		if (!SageInterface::isPointerType(argType))
		{
			std::cerr << "Error: Argument was not a pointer." << std::endl;
			throw 0;
		}
		while (!SageInterface::isScalarType(argType))
			argType = SageInterface::getElementType(argType);	//FIXME what will it to with int** for example ?
		SgScopeStatement* scope = SageInterface::getScope(functionCall);
		
		return SageBuilder::buildOpaqueVarRefExp(argType->unparseToString(), scope);
		
		/*if (isSgTypeInt(argType))
			return SageBuilder::buildOpaqueVarRefExp("int", scope);
		else
		{
			std::cerr << "Error: Unsupported type." << SageInterface::get_name(argType) << std::endl;
			throw 0;
		}*/
	}
	
	SgFunctionCallExp* getFunctionCallExp(SgStatement* functionCall)
	{
		//temp variables for downcasting
		SgExpression* tempExp;
	
		//checking the SgStatement in parameter and extracting func name TODO add !=NULL checking
		SgExprStatement* exprSmt = isSgExprStatement(functionCall);
		SgVariableDeclaration* varDec = isSgVariableDeclaration(functionCall);
		if (exprSmt)
		{
			tempExp = exprSmt->get_expression(); // case f(...) or a = f(...)
			SgAssignOp* assignOp = isSgAssignOp(tempExp);
			if (assignOp) // case a = f(...)
			{
				tempExp = assignOp->get_rhs_operand();
			}
		}
		else if (varDec) // case int a = f(...)
		{
			SgInitializedName* initName = SageInterface::getFirstInitializedName(varDec);
			SgAssignInitializer* assignInit = isSgAssignInitializer(initName->get_initializer());
			if (!assignInit)
			{
				std::cerr << "Error: invalid form for variable declaration." << std::endl;
				throw 0;
			}
			tempExp = assignInit->get_operand();
		}
		else
		{
			std::cerr << "Error: function call has invalid form." << std::endl;
			throw 0;
		}
		SgFunctionCallExp* result = isSgFunctionCallExp(tempExp);
		if (!result)
		{
			std::cerr << "Error: could not find function call in expression." << std::endl;
			throw 0;
		}
		else
			return result;
		
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
	void processArgs(SgStatement* target, Attribute* attribute, SgStatement* functionToMap)
	{
		//we go through the function's parameters
		SgExprListExp* argList = getArgList(functionToMap);
		if (!attribute->checkAll()) //verification of pragma content
				throw 0;
		for (unsigned int i=1; i<=argList->get_expressions().size(); i++) //counting from 1 !
		{
			//these lines include various verifications
			int argIndex = attribute->argIndex(i);	//FIXME currently, missing mapping info provokes an error
			ArgType argType = attribute->argType(argIndex);
			int dimension = attribute->argDimension(argIndex);
			
			//parameters for the smecyAddXXX methods
			SgScopeStatement* scope = SageInterface::getScope(functionToMap);
			SgExpression* mapName = attribute->getMapName(scope);
			SgExpression* mapNumber = attribute->getMapNumber();
			SgExpression* funcToMapExp = getFunctionRef(functionToMap);
			SgExpression* value = getArgRef(functionToMap, i-1);
			
			if (dimension==0) //scalar arg
			{
				SgExpression* typeDescriptor = getArgTypeDescriptor(functionToMap, i-1);
				if (argType==_arg_in or argType==_arg_inout)
					addSmecySendArg(target, mapName, mapNumber, funcToMapExp, i, typeDescriptor, value);
				if (argType==_arg_out or argType==_arg_inout)
					std::cerr << "Warning: argument " << i << " is a scalar with type out or inout." << std::endl;
			}
			
			if (dimension>0) //vector arg
			{
				SgExpression* typeDescriptor = getArgVectorTypeDescriptor(functionToMap, i-1);
				//TODO insert verification of the fact that the vector spans over the last dimension
				SgExpression* argSize = attribute->argSizeExp(i);
				if (argType==_arg_in or argType==_arg_inout)
					addSmecySendArgVector(target, mapName, mapNumber, funcToMapExp, i, typeDescriptor, value, argSize);
				if (argType==_arg_out or argType==_arg_inout)
					addSmecyGetArgVector(target, mapName, mapNumber, funcToMapExp, i, typeDescriptor, value, argSize);
			}
		}
	}
	
	void processReturn(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgStatement* functionToMap)
	{
		//temp variables for downcasting
		SgExpression* tempExp;
	
		//check the function call statement
		SgExprStatement* exprSmt = isSgExprStatement(functionToMap);
		SgVariableDeclaration* varDec = isSgVariableDeclaration(functionToMap);
		if(exprSmt)
		{
			tempExp = exprSmt->get_expression(); // case f(...) or a = f(...)
			SgAssignOp* assignOp = isSgAssignOp(tempExp);
			if (assignOp) // case a = f(...)
			{
				SgType* type = assignOp->get_lhs_operand()->get_type();
				SageInterface::setRhsOperand(assignOp, smecyReturn(target, mapName, mapNumber, getFunctionRef(functionToMap), type));
			}
			else
				SageInterface::removeStatement(functionToMap);
		}
		else if(varDec)
		{
			SgInitializedName* initName = SageInterface::getFirstInitializedName(varDec);
			SgAssignInitializer* assignInit = isSgAssignInitializer(initName->get_initializer());
			if (!assignInit)
			{
				std::cerr << "Error: invalid form for variable declaration." << std::endl;
				throw 0;
			}
			SgType* type = SageInterface::getFirstVarType(varDec);
			assignInit->set_operand_i(smecyReturn(target, mapName, mapNumber, getFunctionRef(functionToMap), type));
		}
		else
		{
			std::cerr << "Error: function call has invalid form." << std::endl;
			throw 0;
		}
	}
	
	/* Top-level function :
	this is the function that should be called to translate smecy pragmas
	into calls to the SMECY API
	*/
	void translateSmecy(SgProject *sageFilePtr)
	{
		//preprocessing
		attachAttributes(sageFilePtr);
		parseExpressions(sageFilePtr);
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
				SgExpression* mapNumber = attribute->getMapNumber();
				SgStatement* funcToMap = SageInterface::getNextStatement(pragmaDeclaration);
				SgScopeStatement* scope = SageInterface::getScope(funcToMap);
				SgExpression* mapName = attribute->getMapName(scope);

				//adding calls to SMECY API
				addSmecySet(pragmaDeclaration, mapName, mapNumber, getFunctionRef(funcToMap));
				processArgs(pragmaDeclaration, attribute, funcToMap);
				addSmecyLaunch(pragmaDeclaration, mapName, mapNumber, getFunctionRef(funcToMap));
				processReturn(pragmaDeclaration, mapName, mapNumber, funcToMap);
				
				//removing pragma declaration TODO free memory
				SageInterface::removeStatement(pragmaDeclaration);
			}
		}
	}
} //namespace smecy

#endif //SMECY_TRANSLATION_CPP
