#ifndef SMECY_AST_CONSTRUCTION_H
#define SMECY_AST_CONSTRUCTION_H

#include "public.h"
#include "smecyAttribute.h"

//================================================================
// Defines functions used during the translation of SMECY programs
//================================================================

namespace smecy
{
	void attachAttributes(SgProject *sageFilePtr);
}

#endif //SMECY_AST_CONSTRUCTION_H
