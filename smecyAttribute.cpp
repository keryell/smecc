#ifndef SMECY_ATTRIBUTE_CPP
#define SMECY_ATTRIBUTE_CPP

#include "smecyAttribute.h"

//====================================================================================
// Implements attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//====================================================================================

namespace smecy
{
//==========================================================================//
// IntExpt
	IntExpr::IntExpr(int intValue) : intValue(intValue), exprValue("")
	{
		this->isIntBool = true;
	}
	
	IntExpr::IntExpr(std::string exprValue) : intValue(0), exprValue(exprValue)
	{
		this->isIntBool = false;
	}

	bool IntExpr::isExpr()
	{
		return !this->isIntBool;
	}
	
	bool IntExpr::isInt()
	{
		return this->isIntBool;
	}
	
	int IntExpr::getInt()
	{
		if (this->isIntBool)
			return this->intValue;
		else
			return 0;
	}
	
	std::string IntExpr::getExpr()
	{
		if (this->isIntBool)
			return "";
		else
			return this->exprValue;
	}

//==========================================================================//
// Attribute
	Attribute::Attribute(std::string mapName, IntExpr mapNumber) : mapName(mapName), mapNumber(mapNumber)
	{
	}

	Attribute* Attribute::currentAttribute ;
	std::vector<IntExpr> Attribute::argSize;
	std::vector<std::pair<IntExpr,IntExpr> > Attribute::argRange;
	int Attribute::argNumber;
	int Attribute::isExprMode = 0;
	std::stringstream Attribute::expr;
	std::pair<IntExpr,IntExpr> Attribute::currentPair;
	IntExpr Attribute::currentIntExpr;
	
	IntExpr Attribute::newIntExpr(std::string expr)
	{
		//TODO should add transformation into a variable name here
		Attribute::currentAttribute->expressionList.push_back(expr);
		return IntExpr(expr);
	}
	
	IntExpr Attribute::newIntExpr(int integer)
	{
		return IntExpr(integer);
	}

	void Attribute::addArg(int argNumber, ArgType argType)
	{
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			//check for existing Arg clause with same number
			if (this->argList[i].argNumber==argNumber)
			{
				this->argList[i].argType=argType;
				return;
			}
		}
		Arg arg;
		arg.argNumber=argNumber;
		arg.argType=argType;
		this->argList.push_back(arg);
	}
	
	void Attribute::addArg(int argNumber, std::vector<IntExpr> argSize)
	{
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			//check for existing Arg clause with same number
			if (this->argList[i].argNumber==argNumber)
			{
				this->argList[i].argSize=argSize;
				return;
			}
		}
		Arg arg;
		arg.argNumber=argNumber;
		arg.argSize=argSize;
		this->argList.push_back(arg);
	}
	
	void Attribute::addArg(int argNumber, std::vector<std::pair<IntExpr,IntExpr> > argRange)
	{
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			//check for existing Arg clause with same number
			if (this->argList[i].argNumber==argNumber)
			{
				this->argList[i].argRange=argRange;
				return;
			}
		}
		Arg arg;
		arg.argNumber=argNumber;
		arg.argRange=argRange;
		this->argList.push_back(arg);
	}
	
	
	std::vector<std::string> Attribute::getExpressionList()
	{
		return this->expressionList;
	}
	
	void Attribute::print()
	{
		std::cout << "Attribute with " << this->argList.size() << " argument clauses." << std::endl;
		std::cout << "\tMapped to " << this->mapName << " n°" << this->mapNumber << "." << std::endl;
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			std::cout << "\t" ;
			this->argList[i].print();
		}
	}

//==========================================================================//
// Arg
	Arg::Arg() : argNumber(-1), argType(_arg_unknown)
	{
	}

	void Arg::print()
	{
		std::cout << "not implemented yet" << std::endl;
	}
} // namespace smecy

std::ostream& operator<<(std::ostream& os, smecy::IntExpr& ie)
{
	if (ie.isInt())
		os << ie.getInt();
	else
		os << ie.getExpr();
	return os;
}

#endif //SMECY_ATTRIBUTE_CPP
