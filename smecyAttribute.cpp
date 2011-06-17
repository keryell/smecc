#ifndef SMECY_ATTRIBUTE_CPP
#define SMECY_ATTRIBUTE_CPP

#include "smecyAttribute.h"

//====================================================================================
// Implements attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//====================================================================================

namespace smecy
{
	Attribute* Attribute::currentAttribute ;
	std::vector<int> Attribute::argSize;
	std::vector<std::pair<int,int> > Attribute::argRange;
	int Attribute::argNumber;
	int Attribute::isExprMode = 0;
	std::stringstream Attribute::expr;
	std::pair<int,int> Attribute::currentPair;

	void Attribute::addClause(Clause clause)
	{
		this->clauseList.push_back(clause);
	}

	Clause::Clause(std::string accelerator, int unitNumber) : type(_map), accelerator(accelerator), unitNumber(unitNumber)
	{
	}

	Clause::Clause(int argNumber, ArgType argType) : type(_arg_type), argNumber(argNumber), argType(argType)
	{
	}

	Clause::Clause(int argNumber, std::vector<int> argSize) : type(_arg_size), argNumber(argNumber), argSize(argSize)
	{
	}

	Clause::Clause(int argNumber, std::vector<std::pair<int,int> > argRange) : type(_arg_range), argNumber(argNumber), argRange(argRange)
	{
	}

	void Attribute::print()
	{
		for (unsigned int i=0; i<this->clauseList.size(); i++)
			this->clauseList[i].print();
	}

	void Clause::print()
	{
		switch(this->type)
		{
			case _map:
				std::cout << "map(" << this->accelerator << "," << this->unitNumber << ")" << std::endl ; break ;
			case _arg_type:
				std::cout << "arg(" << this->argNumber << "," << this->argType << ")" << std::endl ; break ;
			case _arg_size:
				std::cout << "arg(" << this->argNumber << ",size[" << this->argSize.size() << "])" << std::endl ; break ;
			case _arg_range:
				std::cout << "arg(" << this->argNumber << ",range[" << this->argRange.size() << "])" << std::endl ; break ;
			default:
				std::cerr << "Error : invalid clause" << std::endl ;
		}
	}
} // namespace smecy

#endif //SMECY_ATTRIBUTE_CPP
