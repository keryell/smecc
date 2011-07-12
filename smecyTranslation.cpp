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
				//std::cout << "Found pragma string : " << pragmaString << std::endl ;
				//smecy::parseDirective(pragmaString, pragmaDeclaration)->print();
				//TODO handle merging with existing smecy attribute
				//TODO handle syntax errors and print nice error message
				pragmaDeclaration->addNewAttribute("smecy", smecy::parseDirective(pragmaString, pragmaDeclaration));
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
				SgStatement* target = pragmaDeclaration; //SageInterface::getScope(pragmaDeclaration);
				
				//we build a declaration for each of them
				std::ostringstream declarations("");
				for (int i=(int)exprList.size()-1; i>=0; i=i-1)
					declarations << std::endl << "int __smecy__" << i << " = " << exprList[i] << ";" ;
				
				//then we add the declarations before the current position
				if (declarations.str()!="")
					MiddleLevelRewrite::insert(target,declarations.str(),scope,
							MidLevelCollectionTypedefs::BeforeCurrentPosition);
					
				//now, we can collect the expression in the variable declarations...
				for (unsigned int i=0; i<exprList.size(); i++)
				{
					SgVariableDeclaration* decl = isSgVariableDeclaration(SageInterface::getPreviousStatement(pragmaDeclaration));
					if (!decl)
						std::cerr << debugInfo(target) << "error: Found invalid variable declaration while parsing expressions." << std::endl;
					SgInitializedName* initName = SageInterface::getFirstInitializedName(decl);
					SgAssignInitializer* initializer = isSgAssignInitializer(initName->get_initializer());
					if (!initializer)
						std::cerr << debugInfo(target) << "error: Found invalid initializer while parsing expressions." << std::endl;
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

		//since there's no proper builder for functionCallExp, we will extract it from a functionCallStmt FIXME memory
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
			std::cerr << debugInfo(functionCall) << "error: Invalid arg number for extraction." << std::endl;
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
		
		//TEST
		getArraySize(argRef);
		
		if (!SageInterface::isPointerType(argType))
		{
			std::cerr << debugInfo(functionCall) << "error: Argument is not a pointer." << std::endl;
			throw 0;
		}
		while (!SageInterface::isScalarType(argType))
			argType = SageInterface::getElementType(argType);
		SgScopeStatement* scope = SageInterface::getScope(functionCall);
		
		return SageBuilder::buildOpaqueVarRefExp(argType->unparseToString(), scope);
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
				std::cerr << debugInfo(functionCall) << "error: invalid form for variable declaration." << std::endl;
				throw 0;
			}
			tempExp = assignInit->get_operand();
		}
		else
		{
			std::cerr << debugInfo(functionCall) << "error: function call has invalid form." << std::endl;
			throw 0;
		}
		SgFunctionCallExp* result = isSgFunctionCallExp(tempExp);
		if (!result)
		{
			std::cerr << debugInfo(functionCall) << "error: could not find function call in expression." << std::endl;
			throw 0;
		}
		else
			return result;
		
	}

	std::vector<SgExpression*> getArraySize(SgExpression* expression)
	{		
		//first, get the symbols in expression
		std::vector<SgVariableSymbol*> symbolList = SageInterface::getSymbolsUsedInExpression(expression);
		
		//locate the array symbol in expression
		SgArrayType* type = NULL;
		int arrayCount = 0;
		for (unsigned int i=0; i<symbolList.size(); i++)
			if (isSgArrayType(symbolList[i]->get_type()))
			{
				arrayCount++;
				type = isSgArrayType(symbolList[i]->get_type());
			}
		if (arrayCount != 1)
		{
			std::cerr << "error: could not single out array variable among " << arrayCount << std::endl;
			std::cerr << "\texpression is: " << expression->unparseToString() << std::endl;
			throw 0;
		}
		
		//extract dimensions
		std::vector<SgExpression*> result;
		while (type)
		{
			//std::cout << "DEBUG:" << type->get_index()->unparseToString() << std::endl;
			result.push_back(type->get_index());
			type = isSgArrayType(type->get_base_type());
		}
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
		for (unsigned int i=1; i<=argList->get_expressions().size(); i++) //counting from 1 !
		{
			//these lines include various verifications
			ArgType argType = attribute->argType(i);
			int dimension = attribute->argDimension(i);
			
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
					std::cerr << debugInfo(target) << "warning: argument " << i << " is a scalar with type out or inout." << std::endl;
			}
			
			if (dimension>0) //vector arg
			{
				SgExpression* typeDescriptor = getArgVectorTypeDescriptor(functionToMap, i-1);
				SgExpression* argSize = attribute->argSizeExp(i);
				if (argType==_arg_in or argType==_arg_inout)
					addSmecySendArgVector(target, mapName, mapNumber, funcToMapExp, i, typeDescriptor, value, argSize);
				if (argType==_arg_out or argType==_arg_inout)
					addSmecyGetArgVector(target, mapName, mapNumber, funcToMapExp, i, typeDescriptor, value, argSize);
			}
		}
	}
	
	void processReturn(SgStatement* target, Attribute* attribute, SgStatement* functionToMap)
	{
		SgExpression* tempExp;
		SgExpression* mapName = attribute->getMapName(SageInterface::getScope(functionToMap));
		SgExpression* mapNumber = attribute->getMapNumber();
	
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
				std::cerr << debugInfo(target) << "error: invalid form for variable declaration." << std::endl;
				throw 0;
			}
			SgType* type = SageInterface::getFirstVarType(varDec);
			assignInit->set_operand_i(smecyReturn(target, mapName, mapNumber, getFunctionRef(functionToMap), type));
		}
		else
		{
			std::cerr << debugInfo(target) << "error: function call has invalid form." << std::endl;
			throw 0;
		}
	}
	
	void completeSizeInfo(SgStatement* target, Attribute* attribute, SgStatement* functionToMap)
	{
		//we go through the function's parameters
		SgExprListExp* argList = getArgList(functionToMap);
		for (unsigned int i=0; i<argList->get_expressions().size(); i++) //counting from 1 !
		{
			if (!SageInterface::isScalarType(argList->get_expressions()[i]->get_type())) //arg is a vector
			{
				if (attribute->getSize(i+1).size() == 0)
				{
					std::vector<SgExpression*> tentativeSize = getArraySize(argList->get_expressions()[i]);
					if (tentativeSize.size()>0)
					{
						std::vector<IntExpr> newSize;
						for (unsigned int j=0; j<tentativeSize.size(); j++)
							newSize.push_back(IntExpr(tentativeSize[j]));
						//std::cout << "DEBUG: found size for argument in program" << std::endl;
						attribute->getSize(i+1) = newSize;
						//attribute->print();
					}
					else
						std::cerr << debugInfo(target) << "warning: could not find size information for non-scalar argument" << std::endl;
				}
			}
		}
	}
	
	//this function prevents pragmas from being caught by structures and used without block body instead
	//of the function they map
	void correctParentBody(SgProject* sageFilePtr)
	{
		//algorithm: go up in the graph until a SgBasicBlock is encountered
		//then, the next statement is the function call
		
		//first, go through the pragmas
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
				//then, see if there's a potential problem
				if (!isSgBasicBlock(pragmaDeclaration->get_parent()))
				{
					//then, locate the SgBasicBlock
					SgNode* currentNode = pragmaDeclaration->get_parent()->get_parent();
					SgNode* previousNode = pragmaDeclaration->get_parent();
					while (!isSgBasicBlock(currentNode))
					{
						previousNode = currentNode;
						currentNode = currentNode->get_parent();
					}
					
					//then locate next statement
					SgStatementPtrList statements = isSgBasicBlock(currentNode)->get_statements();
					SgStatement* functionCall = NULL;
					for (unsigned int i=0; i<statements.size(); i++)
						if (statements[i] == previousNode and i!=statements.size()-1)
							functionCall = statements[i+1];
					if (!functionCall)
					{
						std::cerr << debugInfo(pragmaDeclaration) << "error: unexpected AST structure or missing function call" << std::endl;
						throw 0;
					}
					//std::cout << "DEBUG: " << functionCall->unparseToString() << std::endl;
					//reorganizes statements correctly
					SgStatement* newFunctionCall = isSgStatement(SageInterface::copyStatement(functionCall));
					SageInterface::ensureBasicBlockAsParent(pragmaDeclaration);
					SageInterface::insertStatement(pragmaDeclaration, newFunctionCall, false);
					SageInterface::removeStatement(functionCall);
				}
			}
		}
	}
	
	bool processCommandLine(int &argc, char** (&argv))
	{
		//getting options in proper form
		std::vector<std::string> list = CommandlineProcessing::generateArgListFromArgcArgv(argc, argv);
		
		//openmp settings
		CommandlineProcessing::removeArgs(list,"-rose:openmp");
		if (CommandlineProcessing::isOption(list,"-fopenmp","",false))
			list.push_back("-rose:openmp:ast_only");
		
		//astRewrite-related settings
		if (!CommandlineProcessing::isOption(list,"--edg:","(no_warnings)",false))
			list.push_back("--edg:no_warnings");
			
		if (!CommandlineProcessing::isOption(list,"--edg","(:c99|=c99)",false) and CommandlineProcessing::isOption(list,"-std","(=c99)",false))
			list.push_back("--edg:c99");
			
		//include and linking smecy lib
		std::vector<std::string>::iterator it;
		std::string lib =  "";
		for (it=list.begin() ; it<list.end(); it++)
			if ((*it).substr(0,12) == "--smecy_lib=")
			{
				//std::cout << "LIB : " << (*it).substr(12) << std::endl;
				lib = (*it).substr(12);
				list.erase(it);
			}
		if (lib=="" and getenv("SMECY_LIB")) //if not in command line, search environment
			lib=getenv("SMECY_LIB");
		if (lib!="" ) 
		{
			std::stringstream concat("");
			concat << "-I" << lib << "/";
			list.push_back(concat.str());
			if (!CommandlineProcessing::isOption(list,"-c","",false))
			{
				concat.str("");
				concat << lib << "/smecy.o";
				list.push_back(concat.str());
			}
		}
		
		bool isSmecy = CommandlineProcessing::isOption(list,"-smecy","",true);
		
		//setting
		argv=NULL;
		CommandlineProcessing::generateArgcArgvFromList(list, argc, argv);
		
		return isSmecy;
	}
	
	void processIf(SgStatement*& target, Attribute* attribute, SgStatement*& functionToMap)
	{
		SgExpression* condition = attribute->getIf();
		if (condition)
		{
			SgStatement* newFunctionCall = isSgStatement(SageInterface::copyStatement(functionToMap));
			SgStatement* newTarget = isSgStatement(SageInterface::copyStatement(target));
			SgStatement* newIf = SageBuilder::buildIfStmt(condition, newFunctionCall, newTarget);
			SageInterface::replaceStatement(target, newIf);
			target = newTarget;
			SageInterface::removeStatement(functionToMap);
			SageInterface::insertStatement(target,functionToMap,false);
		}
	}
	
	void processVariableDeclaration(SgStatement* target, Attribute* attribute, SgStatement*& functionToMap)
	{
		if (isSgVariableDeclaration(functionToMap))
		{
			//preparing parameters
			SgVariableDeclaration* varDec = isSgVariableDeclaration(functionToMap);
			SgInitializedName* initName = SageInterface::getFirstInitializedName(varDec);
			SgAssignInitializer* assignInit = isSgAssignInitializer(initName->get_initializer());
			if (!assignInit)
			{
				std::cerr << debugInfo(target) << "error: invalid form for variable declaration" << std::endl;
				throw 0;
			}
			
			//building assignment alone
			SgExpression* operand = assignInit->get_operand_i();
			SgExpression* varRef = SageBuilder::buildVarRefExp(varDec);
			SgExpression* assignOp = SageBuilder::buildAssignOp(varRef,operand);
			SgStatement* assignment = SageBuilder::buildExprStatement(assignOp);
			
			//now, remove right side of variable declaration
			initName->set_initptr(NULL);
			
			//reorganize statements
			SageInterface::insertStatement(functionToMap, assignment);
			SageInterface::removeStatement(functionToMap);
			SageInterface::insertStatement(target, functionToMap);
			functionToMap = assignment;
		}
	}
	
	/* Helper functions
	*/
	void addBufferVariablesDeclarations(SgScopeStatement* scope, SgStatement* functionCall)
	{
		SgExpressionPtrList argList = getArgList(functionCall)->get_expressions();
		for (unsigned int i=0; i<argList.size(); i++)
		{
			SgVarRefExp* varRef = isSgVarRefExp(argList[i]);
			if (varRef)
			{
				SgType* type = isSgType(SageInterface::deepCopyNode(varRef->get_type()));
				SgName name = varRef->get_symbol()->get_name();
				SgAssignInitializer* initializer = NULL;//SageBuilder::buildAssignInitializer(SageBuilder::buildIntVal(1), type);
				SgVariableDeclaration* varDec = SageBuilder::buildVariableDeclaration(name, type, initializer, scope);
				SageInterface::appendStatement(varDec, scope);
			}
		}
	}
	
	/* Top-level function :
	this is the function that should be called to translate smecy pragmas
	into calls to the SMECY API
	*/
	void translateSmecy(SgProject *sageFilePtr)
	{
		//preprocessing
		correctParentBody(sageFilePtr);
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
				SgStatement* target = isSgStatement(pragmaDeclaration);
				
				//parameters
				smecy::Attribute* attribute = (smecy::Attribute*)target->getAttribute("smecy");
				if (!attribute->checkAll()) //verification of pragma content FIXME FIXME add verifications for clause coherency
					throw 0;
				SgStatement* subject = SageInterface::getNextStatement(target);
				
				if (attribute->getStreamLoop()!=-1)
					translateStreamLoop(target, attribute, subject);
				else if (attribute->hasMapClause() and attribute->getStreamNode()==-1) //if stream node mapping is handled separately
					translateMap(target, attribute, subject);
			}
		}
	}
	
	void translateMap(SgStatement* target, Attribute* attribute, SgStatement* functionToMap)
	{
		//various preprocessing steps
		SageInterface::ensureBasicBlockAsParent(target);
		processVariableDeclaration(target, attribute, functionToMap);
		completeSizeInfo(target, attribute, functionToMap);
		processIf(target, attribute, functionToMap);
		
		//parameters
		SgScopeStatement* scope = SageInterface::getScope(functionToMap);
		SgExpression* mapNumber = attribute->getMapNumber();
		SgExpression* mapName = attribute->getMapName(scope);

		//adding calls to SMECY API
		addSmecySet(target, mapName, mapNumber, getFunctionRef(functionToMap));
		processArgs(target, attribute, functionToMap);
		addSmecyLaunch(target, mapName, mapNumber, getFunctionRef(functionToMap));
		processReturn(target, attribute, functionToMap);

		//removing pragma declaration TODO free memory
		SageInterface::removeStatement(target);
	}
	
	void translateStreamLoop(SgStatement* target, Attribute* attribute, SgStatement* whileLoop)
	{
		if (!isSgWhileStmt(whileLoop))
		{
			std::cerr << debugInfo(target) << "error: target of stream_loop is not a while loop" << std::endl;
			throw 0;
		}
		else
		{
			//getting loop's body and condition
			SgWhileStmt* whileStmt = isSgWhileStmt(whileLoop);
			SgStatement* body = whileStmt->get_body();
			SgBasicBlock* block = isSgBasicBlock(body);
			if (!block)
			{
				std::cerr << debugInfo(target) << "error: while body is a single statement" << std::endl;
				throw 0;
			}
			SgStatementPtrList statements = block->get_statements();
			SgStatement* condition = SageInterface::getLoopCondition(whileStmt);
			
			std::vector<std::pair<SgStatement*,SgStatement*> > streamNodes;
			std::vector<SgExpression*> stream;
			
			//locating the nodes to target
			for (unsigned int i=0; i<statements.size(); i++)
				if (isSgPragmaDeclaration(statements[i]) and i+1!=statements.size())
				{
					//isolating func call and pragma
					std::cerr << "DEBUG: " << statements[i]->unparseToString() << std::endl;
					SgFunctionCallExp* funcCall = NULL;
					if (isSgExprStatement(statements[i+1]))
						funcCall = getFunctionCallExp(statements[i+1]);
					if (!funcCall)
					{
						std::cerr << debugInfo(statements[i+1]) << "error: mapped statement is not a function call" << std::endl;
						throw 0;
					}
					
					//FIXME temporary, need real computation for stream
					stream.clear();
					SgExpressionPtrList argList = getArgList(statements[i+1])->get_expressions();
					for (unsigned int j=0; j<argList.size(); j++)
						stream.push_back(argList[j]);
					//FIXME
					
					streamNodes.push_back(std::pair<SgStatement*,SgStatement*>(statements[i],statements[i+1]));
				}
			
			//calling transformation afterwards since stream computation may depend on all the nodes
			for (unsigned int i=0; i<streamNodes.size(); i++)
			{
				//calling functions to translate
				processStreamNode(streamNodes[i].first, streamNodes[i].second, attribute, stream, i, condition);
			}
		}
	}
	
	void processStreamNode(SgStatement* target, SgStatement* functionToMap, Attribute* parentAttribute, std::vector<SgExpression*> stream, int number,
			SgStatement* condition)
	{	
		//creating function definition with empty body
		SgGlobal* scope = SageInterface::getGlobalScope(target);
		SgType* argType = SageBuilder::buildOpaqueType("DbLink", scope);
		SgType* returnType = SageBuilder::buildVoidType();
		SgInitializedName* param1 = SageBuilder::buildInitializedName("data_buffer", argType);
		SgFunctionParameterList* paramList = SageBuilder::buildFunctionParameterList(param1);
		std::stringstream uniqueName("");
		uniqueName << "__Node_" << number << "_" << parentAttribute->getStreamLoop() ;
		SgFunctionDeclaration* declaration = SageBuilder::buildDefiningFunctionDeclaration(uniqueName.str(), returnType, paramList, scope);
		
		SageInterface::appendStatement(declaration, scope);
		
		//filling the body
		SgFunctionDefinition* definition = declaration->get_definition();
		SgBasicBlock* defBody = SageBuilder::buildBasicBlock(); //FIXME FIXME understand why definition can't handle several statements
		SageInterface::appendStatement(defBody, definition);
		addBufferVariablesDeclarations(defBody, functionToMap);
		//TODO use helper function to fill while body with send/receive to buffer
			//TODO use helper function do get local variables from buffer
		SgStatement* whileLoop = SageBuilder::buildWhileStmt(SageInterface::copyStatement(condition), SageInterface::copyStatement(functionToMap));
		SageInterface::appendStatement(whileLoop, defBody);
	}
	
	/* Other 
	FIXME move to public.cpp
	*/
	std::string debugInfo(SgNode* context)
	{
		if (context)
		{
			std::stringstream ss("");
			ss << context->get_file_info()->get_filenameString() << ":" << context->get_file_info()->get_line() << ": ";
			return ss.str();
		}
		else
			return "";
	}
} //namespace smecy

#endif //SMECY_TRANSLATION_CPP
