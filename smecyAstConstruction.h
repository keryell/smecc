#ifndef SMECY_AST_CONSTRUCTION_H
#define SMECY_AST_CONSTRUCTION_H

#include "public.h"
#include "smecyAttribute.h"

namespace smecy
{
	//TODO decide when it should happen and set parameter accordingly
	void attachSmecyAttributes(SgProject *sageFilePtr);
}

#endif //SMECY_AST_CONSTRUCTION_H
