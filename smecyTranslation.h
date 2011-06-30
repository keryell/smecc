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
	void processReturn(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgStatement* functionToMap);
	void completeSizeInfo(SgStatement* target, Attribute* attribute, SgStatement* functionToMap);
	
	//functionas that get useful AST parts from the function call / attributes
	SgExpression* getFunctionRef(SgStatement* functionCall);
	SgExprListExp* getArgList(SgStatement* functionCall);
	SgExpression* getArgRef(SgStatement* functionCall, int argNumber);
	SgExpression* getArgTypeDescriptor(SgStatement* functionCall, int argNumber);
	SgExpression* getArgVectorTypeDescriptor(SgStatement* functionCall, int argNumber);
	SgFunctionCallExp* getFunctionCallExp(SgStatement* functionCall);
	std::vector<SgExpression*> getArraySize(SgExpression* expression);
	SgExpression* copy(SgExpression* param);
	
	//the top-level translating function
	void translateSmecy(SgProject* sageFilePtr);
}

#endif //SMECY_TRANSLATION_H
