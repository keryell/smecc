#include "smecyTranslation.hpp"
#include <boost/iterator/filter_iterator.hpp>

//===================================================================
// Implements functions used during the translation of SMECY programs
//===================================================================

namespace smecy {
/* \brief Test if the string can describe a #pragma smecy
 */
  bool smecyPragmaString(std::string s) {
    // Get the first word of it by reading from an ad-hoc string stream
    // initialized with the full pragma:
    std::string pragmaHead;
    std::istringstream stream(s);
    stream >> pragmaHead;
    bool is_smecy_pragma = pragmaHead == "smecy";
    //std::cout << "Pragma:" << pragmaHead << "is SMECY:" << is_smecy_pragma << std::endl;
    return is_smecy_pragma;
  }

  struct is_SMECY_pragma {
    bool operator()(SgNode * n) {
      SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(n);
      // Extract the real pragma string:
      std::string pragmaString = pragmaDeclaration->get_pragma()->get_pragma();
      return smecyPragmaString(pragmaString);
    }
  };

  bool is_SMECY_pragma_p(SgNode * n) {
    SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(n);
    // Extract the real pragma string:
    std::string pragmaString = pragmaDeclaration->get_pragma()->get_pragma();
    return smecyPragmaString(pragmaString);
  }

  typedef boost::filter_iterator < is_SMECY_pragma, std::vector<SgNode*>::iterator >
      FilterSmecyPragma;

  std::vector<SgNode*> allFilePragma(SgFile *f) {
    // Making a list of all pragma nodes in AST and going through it
    return NodeQuery::querySubTree(f, V_SgPragmaDeclaration);
  }

  // \brief Brute force approach to get SMECY pragmas as an iterable object
  class SmecyPragma : public std::vector<SgNode*> {
  public:
    SmecyPragma(SgFile *f) : std::vector<SgNode*> {
      /* The vector constructor value is initialized
         with the pragmas à la C++11 mode */
      //NodeQuery::querySubTree(f, V_SgPragmaDeclaration)
    } {
      // This solution does not seems to work. May be SgNode* are not "Move Assignable"? Not clear...
      //      std::remove_if(this->begin(), this->end(), is_SMECY_pragma_p);
      for (auto p: NodeQuery::querySubTree(f, V_SgPragmaDeclaration)) {
	// Only keep SMECY pragmas:
	if (is_SMECY_pragma_p(p))
	  this->push_back(p);
      }
    }
  };

  FilterSmecyPragma smecyPragma(std::vector<SgNode*> pragmas) {
    return boost::make_filter_iterator<is_SMECY_pragma>(pragmas.begin(), pragmas.end());
  }

  FilterSmecyPragma smecyPragmaEnd(std::vector<SgNode*> pragmas) {
    return boost::make_filter_iterator<is_SMECY_pragma>(pragmas.end(), pragmas.end());
  }


  /* \brief First steps : attaching attributes and parsing expressions
     from pragmas.

     TODO : merge so as to go through the pragmas only once ?
  */
  void attachAttributes(SgProject *p) {
    // For all files of the project:
    for (auto * f : p->get_fileList()) {
      // Making a list of all pragma nodes in AST and going through it
      std::vector<SgNode*> allPragmas = NodeQuery::querySubTree(f,
                                                                V_SgPragmaDeclaration);
      for (auto pragma : allPragmas) {
        SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(pragma);
        // Extract the real pragma string:
        std::string pragmaString = pragmaDeclaration->get_pragma()->get_pragma();
        //std::cout << "Found pragma string : " << pragmaString << std::endl ;
        if (smecyPragmaString(pragmaString)) {
          // This is a "#pragma smecy", then dig further...

          //std::cout << "Found smecy pragma string : " << pragmaString << std::endl ;
          //smecy::parseDirective(pragmaString, pragmaDeclaration)->print();
          //TODO handle merging with existing smecy attribute
          //TODO handle syntax errors and print nice error message

          // Attach a parsed version of the pragma to the AST through
          // flex/bison:
          pragmaDeclaration->addNewAttribute("smecy",
                                             smecy::parseDirective(pragmaString, pragmaDeclaration));
        }
      }
    }
  }


  /* \brief parses expressions contained in pragmas and fills
     corresponding Attribute objects with the corresponding
     SgExpression object
  */
  void parseExpressions(SgProject *p) {
    // For all files of the project:
    for (auto * f : p->get_fileList()) {
      // Making a list of all pragma nodes in AST and going through it
      std::vector<SgNode*> allPragmas = NodeQuery::querySubTree(f,
                                                                V_SgPragmaDeclaration);
      for (auto pragma : allPragmas) {
        SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(pragma);
        std::string pragmaString = pragmaDeclaration->get_pragma()->get_pragma();
        if (smecyPragmaString(pragmaString)) {
          // We get the list of all expressions contained in the smecy directive
          auto attribute = (smecy::Attribute*) pragmaDeclaration->getAttribute(
                                                                               "smecy");
          std::vector<std::string> exprList = attribute->getExpressionList();

          // Insert new code into the scope represented by the statement (applies to SgScopeStatements)
          MiddleLevelRewrite::ScopeIdentifierEnum scope =
            MidLevelCollectionTypedefs::SurroundingScope;
          SgStatement* target = pragmaDeclaration; //SageInterface::getScope(pragmaDeclaration);

          // Big hack to be able to evaluate expressions in pragma in local context: we create some dummy variable declarations initialized with the expressions found in the pragma. :-)
          // Then we extract the expression from the declarations and remove the declarations.

          // We build a declaration for each of them
          std::ostringstream declarations { "" };
          for (int i = (int) exprList.size() - 1; i >= 0; i--)
            declarations << std::endl << "int __smecy__" << i << " = "
                         << exprList[i] << ";";

          // Then we add the declarations before the current position if there is something to insert
          if (declarations.str() != "")
            MiddleLevelRewrite::insert(target, declarations.str(), scope,
                                       MidLevelCollectionTypedefs::BeforeCurrentPosition);

          // Now, we can collect the expression in each variable declaration...
          for (unsigned int i = 0; i < exprList.size(); i++) {
            SgVariableDeclaration* decl = isSgVariableDeclaration(
                                                                  SageInterface::getPreviousStatement(pragmaDeclaration));
            if (!decl)
              std::cerr << debugInfo(target)
                        << "error: Found invalid variable declaration while parsing expressions."
                        << " #pragma " << pragmaString << " may be wrong in file "
                        << f->getFileName() << std::endl;

            SgInitializedName* initName = SageInterface::getFirstInitializedName(
                                                                                 decl);
            SgAssignInitializer* initializer = isSgAssignInitializer(
                                                                     initName->get_initializer());
            if (!initializer)
              std::cerr << debugInfo(target)
                        << "error: Found invalid initializer while parsing expressions from"
                        << " #pragma " << pragmaString << " may be wrong in file "
                        << f->getFileName() << std::endl;
            SgExpression* expr = initializer->get_operand();

            // ...store it in the attribute...
            attribute->addParsedExpression(expr);

            // ...and remove the declaration
            SageInterface::removeStatement(decl);
            //TODO remove the rest of the declaration from memory
          }
        }
      }
    }
  }


  /* AddSmecyXXX functions :
     these functions add calls to smecy API to the AST
     TODO : a little refactoring since they all look alike
  */

  /* \brief Add the SME-C specific includes at the beginning of each
     processed source file

     TODO check if the include is already present
  */
  void addSmecyInclude(SgProject *p) {
    // For all files of the project:
    for (auto * f : p->get_fileList()) {
      // Get the beginning of the IR:
      // Mimic the implementation of:
      // SgGlobal * SageInterface::getFirstGlobalScope(SgProject *project)
      SgScopeStatement* scope = isSgSourceFile(f)->get_globalScope();
      SageInterface::insertHeader("p4a_macros.h", PreprocessingInfo::after, false, scope);
      SageInterface::insertHeader("smecy.h", PreprocessingInfo::after, false, scope);
    }
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

/** \brief Generate the call to SMECY_launch(pe, instance, func, n_args)
 *  in the generated code
 *  */
  void addSmecyLaunch(SgStatement* target,
                      SgExpression* mapName,
                      SgExpression* mapNumber,
                      SgStatement* functionToMap) {
    SgExpression* f = getFunctionRef(functionToMap);
    // Build the expression to the number or arguments given to the function to call:
    SgIntVal* nb_args =
        SageBuilder::buildIntVal(getArgList(functionToMap)->get_expressions().size());
    //building parameters to build the func call (bottom-up building)
    SgExprListExp * exprList = SageBuilder::buildExprListExp(copy(mapName), copy(mapNumber), copy(f), copy(nb_args));
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

	/* Functions to add calls to p4a macro calls
	*/
	SgExprStatement* addP4aMacro(std::string name, int arg1, SgScopeStatement* scope)
	{
		//building parameters to build the func call (bottom-up building)
		SgExprListExp * exprList = SageBuilder::buildExprListExp(SageBuilder::buildIntVal(arg1));
		SgName sgName(name);
		SgType* type = SageBuilder::buildVoidType();

		//building the function call
		return SageBuilder::buildFunctionCallStmt(sgName, type, exprList, scope);
	}

	SgExprStatement* addP4aMacro(std::string name, int arg1, int arg2, SgScopeStatement* scope)
	{
		//building parameters to build the func call (bottom-up building)
		SgExprListExp * exprList = SageBuilder::buildExprListExp(SageBuilder::buildIntVal(arg1), SageBuilder::buildIntVal(arg2));
		SgName sgName(name);
		SgType* type = SageBuilder::buildVoidType();

		//building the function call
		return SageBuilder::buildFunctionCallStmt(sgName, type, exprList, scope);
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

	/* get a specific arg reference from arg number argNumber in
	function call functionCall*/
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

	/* get a specific type descriptor from arg number argNumber in
	function call functionCall
	a type descriptor is actually a fake variable named like the type
	(like "int" or "unsigned short")*/
	SgExpression* getArgTypeDescriptor(SgStatement* functionCall, int argNumber)
	{
		SgExpression* argRef = getArgRef(functionCall, argNumber);
		SgType* argType = argRef->get_type();
		SgScopeStatement* scope = SageInterface::getScope(functionCall);

		return SageBuilder::buildOpaqueVarRefExp(argType->unparseToString(), scope);
	}

	/* get a specific type descriptor from arg number argNumber in
	function call functionCall
	a type descriptor is actually a fake variable named like the type
	(like "int" or "unsigned short")
	here, if argument is a pointer, the type descriptor returned will be the
	"base type" of the pointer ex: "int" for a "int**"*/
	SgExpression* getArgVectorTypeDescriptor(SgStatement* functionCall, int argNumber)
	{
		SgExpression* argRef = getArgRef(functionCall, argNumber);
		SgType* argType = argRef->get_type();

		if (SageInterface::isScalarType(argType))
		{
			//std::cerr << "DEBUG: " << argType->unparseToString() << std::endl;
			std::cerr << debugInfo(functionCall) << "error: Argument is a scalar and the cannot be used to pass information between pipeline stages (args are passed by copy in C...)." << std::endl;
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

	/*this function adds the call to smecyReturn where needed and if needed*/
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
		else if(varDec) // case int a = f(...)
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

	//this functions tries to find the size of arrays in the code if it is not specified in pragma
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

	// \brief This function prevents pragmas from being caught by structures and used without block body instead
	// of the function they map
	void correctParentBody(SgFile* f) {
	  // Algorithm: go up in the graph until a SgBasicBlock is encountered,
	  // then, the next statement is the function call

	  // First, go through the pragmas
	  for(auto pragma : SmecyPragma(f)) {
	    SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(pragma);
	    // Then, see if there's a potential problem
        if (!isSgBasicBlock(pragmaDeclaration->get_parent())) {
          // Then, locate the SgBasicBlock
          SgNode* currentNode = pragmaDeclaration->get_parent()->get_parent();
          SgNode* previousNode = pragmaDeclaration->get_parent();
          while (!isSgBasicBlock(currentNode)) {
            previousNode = currentNode;
            currentNode = currentNode->get_parent();
          }

          // Then locate next statement
          SgStatementPtrList statements = isSgBasicBlock(currentNode)->get_statements();
          SgStatement* functionCall = NULL;
          for (unsigned int i=0; i<statements.size(); i++)
            if (statements[i] == previousNode and i!=statements.size()-1)
              functionCall = statements[i+1];
          if (!functionCall) {
            std::cerr << debugInfo(pragmaDeclaration) << "error: unexpected AST structure or missing function call" << std::endl;
            throw 0;
          }
          // std::cout << "DEBUG: " << functionCall->unparseToString() << std::endl;
          // Reorganizes statements correctly
          SgStatement* newFunctionCall = isSgStatement(SageInterface::copyStatement(functionCall));
          // This does no longer exist in recent ROSE Compiler:
	  //SageInterface::ensureBasicBlockAsParent(pragmaDeclaration);
          SageInterface::insertStatement(pragmaDeclaration, newFunctionCall, false);
          SageInterface::removeStatement(functionCall);
        }
	  }
	}


	//this function should be used to process the compiler's command line
	//it makes various modifications to allow an easier use
	bool processCommandLine(int &argc, char** (&argv))
	{
	  std::cerr << "Options before processing:";
	  for(int i = 0; i < argc; i++)
	    std::cerr << " " << argv[i];
	  std::cerr << std::endl;


		// Getting options in a string vector instead of argc/argv fo easier later processing:
		std::vector<std::string> list = CommandlineProcessing::generateArgListFromArgcArgv(argc, argv);
#if 0
		//openmp settings
		CommandlineProcessing::removeArgs(list,"-rose:openmp");
		if (CommandlineProcessing::isOption(list,"-fopenmp","",false))
			list.push_back("-rose:openmp:ast_only");

		//astRewrite-related settings
		if (!CommandlineProcessing::isOption(list,"--edg:","(no_warnings)",false))
			list.push_back("--edg:no_warnings");

		if (!CommandlineProcessing::isOption(list,"--edg","(:c99|=c99)",false) and CommandlineProcessing::isOption(list,"-std","(=c99)",false))
			list.push_back("--edg:c99");
#endif
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

		// Test if the -smecy option is here and remove it from the list:
		bool isSmecy = CommandlineProcessing::isOption(list,"-smecy","",true);

		std::cerr << "Options after processing:" << list << std::endl;

		//setting
		argv=NULL;
		// Generate back an updated arc/argv argument list:
		CommandlineProcessing::generateArgcArgvFromList(list, argc, argv);

		return isSmecy;
	}

	//is there is a if clause in a pragma, this function reorganizes the code to add the if
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

	//if the function call to map is a declaration, then the declaration should be moved
	//before the pragma in case there is an if clause (so that the variable is declared
	//in the right scope)
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

  /* Create a variable named p4a_struct_buffer of type struct p4a_buffer_type_<n>
   * where n is the stream_loop number
   */
  void addBufferVariablesDeclarations(int nLoop, SgScopeStatement* scope,
                                      SgStatement* functionCall,
                                      std::string varName) {
    // Construct the type including the loop number:
    std::stringstream ss("");
    ss << "struct p4a_buffer_type_" << nLoop;
    SgType* type = SageBuilder::buildPointerType(SageBuilder::buildOpaqueType(ss.str(), scope));
    SgVariableDeclaration* varDec =
        SageBuilder::buildVariableDeclaration(SgName { varName },
                                              type,
                                              (SgAssignInitializer *) nullptr,
                                              scope);
    SageInterface::appendStatement(varDec, scope);
  }


  /* \brief define type p4a_buffer_type_<n>* where n is the stream_loop number

	   this is a struct containing all the variables forming the stream of data
	   using a struct allows easy recuperation of data in the stream after a cast has been done
	 */
  void addBufferTypedef(Attribute* attribute,
                        std::vector<SgExpression*> stream,
                        SgScopeStatement* scope) {
    // Build the struct name:
    std::stringstream ss { "" };
    ss << "p4a_buffer_type_" << attribute->getStreamLoop();
    // Add the struct declaration and definition:
    SgClassDeclaration* structDecl = SageBuilder::buildStructDeclaration(ss.str(), scope);
    SgClassDefinition* structDef = SageBuilder::buildClassDefinition(structDecl);
    // Add to the type all the members to pass data between pipeline stages:
    for (size_t i = 0; i < stream.size(); i++) {
      //TODO refactor with addBufferVariablesDeclarations
      SgVarRefExp* varRef = isSgVarRefExp(stream[i]);
      // TODO there is currently a restriction on the arguments to be only variable references:
      if (varRef) {
        // Same type as the argument:
        SgType* type = isSgType(SageInterface::deepCopyNode(varRef->get_type()));
        // Same member name as the argument:
        SgName name = varRef->get_symbol()->get_name();
        // No initializer:
        SgAssignInitializer* initializer = nullptr;
        //SageBuilder::buildAssignInitializer(SageBuilder::buildIntVal(1), type);
        // Create the member declaration and add it to the structure:
        SgVariableDeclaration* varDec = SageBuilder::buildVariableDeclaration(name, type, initializer, structDef);
        SageInterface::appendStatement(varDec, structDef);
      }
      else
        std::cerr << "warning: stream function parameter '"
                  << stream[i]->unparseToString()
                  << "' is not a variable reference" << std::endl;
    }
    // Instert the struct declaration at the top level:
    structDecl->set_definition(structDef);
    SgStatement* mainStatement = SageInterface::findMain(SageInterface::getGlobalScope(scope));
    SageInterface::insertStatement(mainStatement, structDecl);
  }


	//constructs the content of the while body contained in a stream_loop associated function
	SgStatement* buildNodeWhileBody(SgStatement* functionToMap, int nLoop, int nNode, SgScopeStatement* scope, bool in, bool out, SgStatement* pragma)
	{
		//body where statements will be added
		SgBasicBlock* body = SageBuilder::buildBasicBlock();

		//now, we create a modified function call
		SgStatement* funcCall = SageInterface::copyStatement(functionToMap);
		SgExprListExp* argList = getArgList(funcCall);
		SgExpressionPtrList& args = argList->get_expressions();
		for (unsigned int i=0; i<args.size(); i++)
		{
			SgVarRefExp* varRef;
			if (out)
				varRef = SageBuilder::buildVarRefExp("p4a_struct_buffer_out", scope);
			else
				varRef = SageBuilder::buildVarRefExp("p4a_struct_buffer_in", scope);
			SgExpression* dot = SageBuilder::buildArrowExp(varRef, args[i]);
			args[i] = dot;
		}

		//adding statements to the body
		if (in) //not the first node, we need to copy the input buffer to the output
			SageInterface::appendStatement(addP4aMacro("P4A_stream_get_data", nLoop, nNode-1, scope), body);
		if (in and out)
			SageInterface::appendStatement(addP4aMacro("P4A_stream_copy_data", nLoop, nNode, scope), body);
		SageInterface::appendStatement(funcCall, body);
		if (out)
			SageInterface::appendStatement(addP4aMacro("P4A_stream_put_data", nLoop, nNode, scope), body);

		//if there is a map clause, process it
		Attribute* attribute = (Attribute*)pragma->getAttribute("smecy");
		if (attribute->hasMapClause()) //if stream node mapping is handled separately
		{
			SgStatement* newPragma = SageInterface::copyStatement(pragma);
			SageInterface::insertStatement(funcCall, newPragma);
			newPragma->removeAttribute("smecy"); //copy does not deep copy attributes well
			newPragma->addNewAttribute("smecy", attribute);
			//translateMap(newPragma, attribute, funcCall);
		}

		return body;
	}

  /* \brief Top-level function this is the function that should be called
     to translate smecy pragmas into calls to the SMECY API
  */
  void translateSmecy(SgProject *p) {
    /* Preprocessing of the project.

       Have each phase working on all the files, so that the phases
       advance lock-step */
    attachAttributes(p);
    parseExpressions(p);
    addSmecyInclude(p);

    // Translating the different kinds of pragmas
    translateStreaming(p);
    translateMapping(p);
  }


  /* \brief

   */
  void translateStreaming(SgProject *p) {
    // For all files of the project:
    for (auto * f : p->get_fileList()) {
      // Preprocessing
      correctParentBody(f);
      for(auto pragma : SmecyPragma(f)) {
        //for(auto pragma : SmecyPragma(f)) {
        SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(pragma);
        SgStatement* target = isSgStatement(pragmaDeclaration);
        //parameters
        smecy::Attribute* attribute = (smecy::Attribute*)target->getAttribute("smecy");
        if (!attribute->checkAll()) //verification of pragma content FIXME FIXME add verifications for clause coherency
          throw 0;
        if (attribute->getStreamLoop()!=-1) {
          // There is a #pragma smecy stream_loop on this loop...
          SgStatement* subject = SageInterface::getNextStatement(target);
          // ... so translate it:
          translateStreamLoop(target, attribute, subject);
        }
      }
    }
  }

  /* \brief Generate code for the mapping #pragma
   *
   */
  void translateMapping(SgProject *p) {
    // For all files of the project:
    for (auto * f : p->get_fileList()) {
      // Preprocessing
      correctParentBody(f);

      for(auto pragma : SmecyPragma(f)) {
        SgPragmaDeclaration* pragmaDeclaration = isSgPragmaDeclaration(pragma);
        SgStatement* target = isSgStatement(pragmaDeclaration);
        // Get the pragma parameters:
        smecy::Attribute* attribute = (smecy::Attribute*)target->getAttribute("smecy");
        // Verification of pragma content FIXME FIXME add verifications for clause coherency:
        if (!attribute->checkAll())
          throw 0;
        SgStatement* subject = SageInterface::getNextStatement(target);

        if (attribute->hasMapClause()) // If stream node mapping is handled separately
          translateMap(target, attribute, subject);
      }
    }
  }


	// If directive has a map clause, translates it in corresponding calls to SMECY API
	void translateMap(SgStatement* target, Attribute* attribute, SgStatement* functionToMap)
	{
		//various preprocessing steps
	        // This does no longer exist in recent ROSE Compiler:
		//SageInterface::ensureBasicBlockAsParent(target);
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
		addSmecyLaunch(target, mapName, mapNumber, functionToMap);
		processReturn(target, attribute, functionToMap);

		//removing pragma declaration TODO free memory
		SageInterface::removeStatement(target);
  }

  /* Try translation of a #pragma stream_loop clause if any along with its loop body
      its while loop containing nodes) into SMECY API
   */
  void translateStreamLoop(SgStatement* target, Attribute* attribute, SgStatement* whileLoop)	{
    // At least verify we have the correct loop statement that can be translated:
    if (!isSgWhileStmt(whileLoop)) {
      std::cerr << debugInfo(target) << "error: target of stream_loop is not a while loop" << std::endl;
      throw 0;
    }
    else {
      // Getting loop's body and condition:
      SgWhileStmt* whileStmt = isSgWhileStmt(whileLoop);
      SgStatement* body = whileStmt->get_body();
      SgBasicBlock* block = isSgBasicBlock(body);

      // Verify we have a basic block as loop body:
      if (!block) {
        std::cerr << debugInfo(target) << "error: while body is a single statement instead of a block" << std::endl;
        throw 0;
      }
      SgStatementPtrList statements = block->get_statements();
      SgStatement* condition = SageInterface::getLoopCondition(whileStmt);

      // Store the boundaries of all the pipeline stages, as couple of (previous,next) statements:
      std::vector<std::pair<SgStatement*,SgStatement*> > streamNodes;
      // The arguments to move through the pipeline:
      // TODO: this should be a set
      std::vector<SgExpression*> stream;

      // Locate and split each stage of the loop to be pipelined
      for (size_t i = 0; i< statements.size(); i++) {
        //std::cerr << "DEBUG: " << statements[i]->unparseToString() << std::endl;
        if (isSgPragmaDeclaration(statements[i]) and i+1!=statements.size()) {
          /* There is a pragma here.
	           Isolate the pragma and the matching function call */

          SgFunctionCallExp* funcCall = nullptr;
          if (isSgExprStatement(statements[i+1]))
            funcCall = getFunctionCallExp(statements[i+1]);
          if (!funcCall) {
            std::cerr << debugInfo(statements[i+1]) << "error: the statement after the #pragma is not a function call" << std::endl;
            throw 0;
          }
          // Get the arguments of the function call:
          SgExpressionPtrList argList = getArgList(statements[i+1])->get_expressions();
          // Add the arguments variables to stream if not already in:
          for (size_t j = 0; j < argList.size(); j++) {
            bool present = false;
            for (size_t ii = 0; ii < stream.size(); ii++) {
              std::cerr << stream[ii]->unparseToString() << '(' << stream[ii] << ')'
                  << argList[j]->unparseToString() << '(' << argList[j] << ')'
                  << std::endl;
              /* FIXME unparseToString should be replaced by a better comparison.
	               We have to compare 2 reference expressions to have the same semantics.
	               The same argument used in 2 different functions is indeed a different SgExpression.
	               Right now, use their string representation as a comparison.
	               But this do not work for example to prove that b[0] and *b are the same, for example.
               */
              present |= stream[ii]->unparseToString() == argList[j]->unparseToString();
            }
            if (!present)
              stream.push_back(argList[j]);
          }
          // Append the current pipeline stage:
          streamNodes.push_back(std::pair<SgStatement*,SgStatement*> {
            statements[i], statements[i+1] });
        }
      }
      // Declaring the type for the buffer used to move information
      // between the pipeline stages:
      addBufferTypedef(attribute, stream, SageInterface::getGlobalScope(target));

      //calling transformation afterwards since stream computation may depend on all the nodes
      for (size_t i = 0; i < streamNodes.size(); i++) {
        // Use some heuristic to guess the information flow:
        // By default a stage can both consume and produce data
        ArgType inout = _arg_inout;
        if (i == 0)
          // But the first stage of the pipeline can only produce data:
          inout = _arg_out;
        else if (i == streamNodes.size()-1)
          // And the last stage of the pipeline can only consume data:
          inout = _arg_in;

        // Outline the each function in the stream as a pipeline stage:
        processStreamNode(streamNodes[i].first,
                          streamNodes[i].second,
                          attribute->getStreamLoop(),
                          i, condition, inout);
      }

      //buffer declaration
      //addGlobalBufferDeclaration(target, attribute);
      SgScopeStatement* scope = SageInterface::getScope(target);
      // Insert the macro to start a pipeline computation with the loop number
      // and the number of stages as parameters:
      SageInterface::insertStatement(target,
                                     addP4aMacro("P4A_init_stream",
                                                 attribute->getStreamLoop(),
                                                 streamNodes.size()-1,
                                                 scope));

      // Add the thread launching for each pipeline stage with the loop number
      // and the number of stages as parameter:
      for (size_t i = 0; i < streamNodes.size(); i++)
        //addThreadCreation(target, attribute, i);
        SageInterface::insertStatement(target,
                                       addP4aMacro("P4A_launch_stream",
                                                   attribute->getStreamLoop(),
                                                   i,
                                                   scope));

      // Insert the function to wait the end of the whole application:
      SgName name { "P4A_wait_for_the_end" };
      SgType* returnType = SageBuilder::buildVoidType();
      SgExprListExp * exprList = SageBuilder::buildExprListExp();
      SgExprStatement* pause = SageBuilder::buildFunctionCallStmt(name, returnType, exprList, scope);
      SageInterface::insertStatement(target, pause);

      // Remove the #pragma and original loop body which has been replaced
      // by the pipeline macros & functions
      SageInterface::removeStatement(target);
      SageInterface::removeStatement(whileStmt);
    }
  }


  /* Translate a single stream_node into low-level SMECY API
     by outlining the pipeline-stage to a new associated function)
   */
  void processStreamNode(SgStatement* target, SgStatement* functionToMap, int nLoop,
                         int nNode, SgStatement* condition, ArgType inout) {
    bool argIn = (inout==_arg_in or inout==_arg_inout);
    bool argOut = (inout==_arg_out or inout==_arg_inout);

    // Create a function definition with an empty body:
    SgGlobal* scope = SageInterface::getGlobalScope(target);
    SgType* returnType = SageBuilder::buildVoidType();
    SgFunctionParameterList* paramList = SageBuilder::buildFunctionParameterList();
    // Name the new function from the loop id and its order in the pipeline"
    std::stringstream uniqueName{ "" };
    uniqueName << "p4a_node_" << nLoop << "_" << nNode;
    SgFunctionDeclaration* declaration =
        SageBuilder::buildDefiningFunctionDeclaration(uniqueName.str(),
                                                      returnType,
                                                      paramList, scope);

    // Inserting the p4a_node* function declaration just before the main:
    SgStatement* mainStatement = SageInterface::findMain(scope);
    SageInterface::insertStatement(mainStatement, declaration);

    // Construct the loop body from the original instruction to stream:
    SgFunctionDefinition* definition = declaration->get_definition();
    SgBasicBlock* defBody = SageBuilder::buildBasicBlock();
    //FIXME FIXME understand why definition can't handle several statements
    SageInterface::appendStatement(defBody, definition);

    // Add a buffer for copy-in data if any:
    if (argIn)
      addBufferVariablesDeclarations(nLoop, defBody, functionToMap,
                                     "p4a_struct_buffer_in");

    // Add a buffer and its initialization for copy-out data if any:
    if (argOut) {
      addBufferVariablesDeclarations(nLoop, defBody, functionToMap,
                                     "p4a_struct_buffer_out");
      SageInterface::appendStatement(addP4aMacro("P4A_stream_get_init_buf",
                                     nLoop, nNode, defBody),
                                     defBody);
    }

    // And finish by adding the infinite while loop around the task:
    SgStatement* whileBody = buildNodeWhileBody(functionToMap, nLoop, nNode,
                                                defBody, argIn, argOut, target);
    SgStatement* whileLoop =
        SageBuilder::buildWhileStmt(SageInterface::copyStatement(condition),
                                    whileBody);
    SageInterface::appendStatement(whileLoop, defBody);
  }

} //namespace smecy
