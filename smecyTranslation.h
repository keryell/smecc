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
	void attachAttributes(SgProject *sageFilePtr);
	
	//extracts all C expressions from smecy pragmas and
	//defines them before the pragma using arbitrary text
	//FIXME non-unique variable name
	void extractExpressions(SgProject *sageFilePtr);
	
	//transforms all C expressions into their SageIII reprsentation
	//using the high level string interface of the ASR rewrite mechanism
	void parseExpressions(SgProject *sageFilePtr);
	
	//replaces smecy pragma nodes with decoration with SgSmecyDirective nodes
	//void buildSmecyNodes(SgProject *sageFilePtr);
}

#endif //SMECY_TRANSLATION_H
