#ifndef SMECY_ATTRIBUTE_CPP
#define SMECY_ATTRIBUTE_CPP

#include "smecyAttribute.h"

//====================================================================================
// Implements attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//====================================================================================

namespace smecy
{
	intExpr::intExpr(int intValue) : intValue(intValue), exprValue("")
	{
		this->isIntBool = true;
	}
	
	intExpr::intExpr(std::string exprValue) : intValue(0), exprValue(exprValue)
	{
		this->isIntBool = false;
	}

	bool intExpr::isExpr()
	{
		return !this->isIntBool;
	}
	
	bool intExpr::isInt()
	{
		return this->isIntBool;
	}
	
	int intExpr::getInt()
	{
		if (this->isIntBool)
			return this->intValue;
		else
			return 0;
	}
	
	std::string intExpr::getExpr()
	{
		if (this->isIntBool)
			return "";
		else
			return this->exprValue;
	}

	Attribute* Attribute::currentAttribute ;
	std::vector<intExpr> Attribute::argSize;
	std::vector<std::pair<intExpr,intExpr> > Attribute::argRange;
	intExpr Attribute::argNumber;
	int Attribute::isExprMode = 0;
	std::stringstream Attribute::expr;
	std::pair<intExpr,intExpr> Attribute::currentPair;
	intExpr Attribute::currentIntExpr;

	void Attribute::addClause(Clause clause)
	{
		this->clauseList.push_back(clause);
	}

	Clause::Clause(std::string accelerator, intExpr unitNumber) : type(_map), accelerator(accelerator), unitNumber(unitNumber)
	{
	}

	Clause::Clause(intExpr argNumber, ArgType argType) : type(_arg_type), argNumber(argNumber), argType(argType)
	{
	}

	Clause::Clause(intExpr argNumber, std::vector<intExpr> argSize) : type(_arg_size), argNumber(argNumber), argSize(argSize)
	{
	}

	Clause::Clause(intExpr argNumber, std::vector<std::pair<intExpr,intExpr> > argRange) : type(_arg_range), argNumber(argNumber), argRange(argRange)
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
				std::cout << "arg(" << this->argNumber << ",";
				for (unsigned int i=0; i<this->argSize.size(); i++)
					std::cout << "[" << this->argSize[i] << "]";
				std::cout << ")" << std::endl ; break ;
			case _arg_range:
				std::cout << "arg(" << this->argNumber << ",/";
				for (unsigned int i=0; i<this->argRange.size(); i++)
					std::cout << "[" << this->argRange[i].first << ":" << this->argRange[i].second << "]";
				std::cout << ")" << std::endl ; break ;
			default:
				std::cerr << "Error : invalid clause" << std::endl ;
		}
	}
} // namespace smecy

std::ostream& operator<<(std::ostream& os, smecy::intExpr& ie)
{
	if (ie.isInt())
		os << ie.getInt();
	else
		os << ie.getExpr();
	return os;
}

#endif //SMECY_ATTRIBUTE_CPP
