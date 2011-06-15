#ifndef SMECY_ATTRIBUTE_CPP
#define SMECY_ATTRIBUTE_CPP

#include "smecyAttribute.h"

//====================================================================================
// Implements attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//====================================================================================

smecyAttribute* smecyAttribute::attributeBeingBuilt ;
std::vector<int> smecyAttribute::argSize;
std::vector<std::pair<int,int> > smecyAttribute::argRange;

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

void smecyAttribute::print()
{
	for (unsigned int i=0; i<this->clauseList.size(); i++)
		this->clauseList[i].print();
}

void smecyClause::print()
{
	switch(this->type)
	{
		case smecy_map:
			std::cout << "map(" << this->accelerator << "," << this->unitNumber << ")" << std::endl ; break ;
		case smecy_arg_type:
			std::cout << "arg(" << this->argNumber << "," << this->argType << ")" << std::endl ; break ;
		case smecy_arg_size:
			std::cout << "arg(" << this->argNumber << ",size[" << this->argSize.size() << "])" << std::endl ; break ;
		case smecy_arg_range:
			std::cout << "arg(" << this->argNumber << ",range[" << this->argRange.size() << "])" << std::endl ; break ;
		default:
			std::cerr << "Error : invalid clause" << std::endl ;
	}
}

#endif //SMECY_ATTRIBUTE_CPP
