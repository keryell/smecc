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
	
	//extracts all C expressions from smecy pragmas and
	//defines them before the pragma using arbitrary text
	//FIXME non-unique variable name
	void extractExpressions(SgProject* sageFilePtr);
	
	//transforms all C expressions into their SageIII representation
	//using the high level string interface of the ASR rewrite mechanism
	void parseExpressions(SgProject* sageFilePtr);
	
	//replaces smecy pragma nodes with decoration with SgSmecyDirective nodes
	//void buildSmecyNodes(SgProject *sageFilePtr);
	
	//functions that add SMECY API functions
	void addSmecyInclude(SgProject* sageFilePtr);
	void addSmecySet(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap);
	void addSmecySendArg(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value);
	void processSendArgs(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap, Attribute* attribute);
	void addSmecyGetArg(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap,
			int argNumber, SgExpression* typeDescriptor, SgExpression* value);
	void processGetArgs(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap, Attribute* attribute);
	void addSmecyLaunch(SgStatement* target, SgExpression* mapName, SgExpression* mapNumber, SgExpression* functionToMap);
	
	//functionas that get useful AST parts from the function call / attributes
	SgExpression* getFunctionRef(SgStatement* functionCall);
	SgExpression* getArgRef(SgStatement* functionCall, int argNumber);
	SgExpression* getArgTypeDescriptor(SgStatement* functionCall, int argNumber);
	SgExpression* getArgVectorTypeDescriptor(SgStatement* functionCall, int argNumber);
	SgExpression* copy(SgExpression* param);
	
	//the top-level translating function
	void translateSmecy(SgProject* sageFilePtr);
}

#endif //SMECY_TRANSLATION_H
