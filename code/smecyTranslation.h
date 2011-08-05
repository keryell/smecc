#ifndef SMECY_TRANSLATION_H
#define SMECY_TRANSLATION_H

#include "public.h"
#include "smecyAttribute.h"

//================================================================
// Defines functions used during the translation of SMECY programs
//================================================================

namespace smecy
{
	//attaches smecy attributes to smecy pragmas
	void attachAttributes(SgProject* sageFilePtr);
	
	//transforms all C expressions into their SageIII representation
	//using the high level string interface of the ASR rewrite mechanism
	void parseExpressions(SgProject* sageFilePtr);
	
	//functions that add SMECY API functions
	void addSmecyInclude(SgProject* sageFilePtr);
	void addSmecySet(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap);
	void addSmecySendArg(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value);
	void addSmecySendArgVector(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value, SgExpression* size);
	void addSmecyGetArgVector(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value, SgExpression* size);
	void addSmecyLaunch(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap);
	SgExpression* smecyReturn(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap, SgType* returnType);
	
	//functions that add p4a macro calls
	SgExprStatement* addP4aMacro(std::string name, int arg1, SgScopeStatement* scope);
	SgExprStatement* addP4aMacro(std::string name, int arg1, int arg2, SgScopeStatement* scope);
	
	//high-level functions
	void processArgs(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void processReturn(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void completeSizeInfo(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void correctParentBody(SgProject* sageFilePtr);
	bool processCommandLine(int &argc, char** (&argv));
	void processIf(SgStatement*& target, Attribute* attribute, SgStatement*& functionToMap);
	void processVariableDeclaration(SgStatement* target, Attribute* attribute, SgStatement*& functionToMap);
	void processStreamNode(SgStatement* target, SgStatement* functionToMap, int nLoop,
			int nNode, SgStatement* condition, ArgType inout);
	
	//functions that get useful AST parts from the function call / attributes
	SgExpression* getFunctionRef(SgStatement* functionCall);
	SgExprListExp* getArgList(SgStatement* functionCall);
	SgExpression* getArgRef(SgStatement* functionCall, int argNumber);
	SgExpression* getArgTypeDescriptor(SgStatement* functionCall, int argNumber);
	SgExpression* getArgVectorTypeDescriptor(SgStatement* functionCall, int argNumber);
	SgFunctionCallExp* getFunctionCallExp(SgStatement* functionCall);
	std::vector<SgExpression*> getArraySize(SgExpression* expression);
	SgExpression* copy(SgExpression* param);
	
	//helper functions
	void addBufferVariablesDeclarations(int nLoop, SgScopeStatement* scope, SgStatement* functionCall);
	void addBufferTypedef(Attribute* attribute, std::vector<SgExpression*> stream, SgScopeStatement* scope);
	SgStatement* buildNodeWhileBody(SgStatement* functionToMap, int nLoop, int nNode, SgScopeStatement* scope, bool in, bool out);
	
	//top-level translating functions
	void translateSmecy(SgProject* sageFilePtr);
	void translateMap(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void translateStreamLoop(SgStatement* target, Attribute* attribute, SgStatement* whileLoop);
}

#endif //SMECY_TRANSLATION_H
