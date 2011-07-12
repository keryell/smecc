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
	
	//high-level functions
	void processArgs(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void processReturn(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void completeSizeInfo(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void correctParentBody(SgProject* sageFilePtr);
	bool processCommandLine(int &argc, char** (&argv));
	void processIf(SgStatement*& target, Attribute* attribute, SgStatement*& functionToMap);
	void processVariableDeclaration(SgStatement* target, Attribute* attribute, SgStatement*& functionToMap);
	void processStreamNode(SgStatement* target, SgStatement* functionToMap, Attribute* parentAttribute, std::vector<SgExpression*> stream,
			int number, SgStatement* condition);
	
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
	void addBufferVariablesDeclarations(SgScopeStatement* scope, SgStatement* functionCall);
	
	//top-level translating functions
	void translateSmecy(SgProject* sageFilePtr);
	void translateMap(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	void translateStreamLoop(SgStatement* target, Attribute* attribute, SgStatement* whileLoop);
}

#endif //SMECY_TRANSLATION_H
