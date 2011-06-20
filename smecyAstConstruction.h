#ifndef SMECY_AST_CONSTRUCTION_H
#define SMECY_AST_CONSTRUCTION_H

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
	//defines them before the pragma
	//FIXME non-unique variable name
	void extractExpressions(SgProject *sageFilePtr);
	
	//replaces smecy pragma nodes with decoration with SgSmecyDirective nodes
	void buildSmecyNodes(SgProject *sageFilePtr);
	
	//required for extractExpression
	class expressionExtractor : public SgSimpleProcessing
	{
	public:
		void visit(SgNode* astNode);
	};
}

#endif //SMECY_AST_CONSTRUCTION_H
