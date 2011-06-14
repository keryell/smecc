#ifndef SMECY_ATTRIBUTE_CPP
#define SMECY_ATTRIBUTE_CPP

#include "smecyAttribute.h"

//====================================================================================
// Implements attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//====================================================================================

void smecyAttribute::addSmecyClause(smecyClause clause)
{
	this->clauseList.push_back(clause);
}

smecyClause::smecyClause(std::string accelerator, int unitNumber) : type(smecy_map), accelerator(accelerator), unitNumber(unitNumber)
{
}

smecyClause::smecyClause(int argNumber, smecyArgType argType) : type(smecy_arg_type), argNumber(argNumber), argType(argType)
{
}

smecyClause::smecyClause(int argNumber, std::vector<int> argSize) : type(smecy_arg_size), argNumber(argNumber), argSize(argSize)
{
}

smecyClause::smecyClause(int argNumber, std::vector<std::pair<int,int> > argRange) : type(smecy_arg_range), argNumber(argNumber), argRange(argRange)
{
}

#endif //SMECY_ATTRIBUTE_CPP
