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
	
	bool IntExpr::isMinus1()
	{
		return (this->isIntBool and this->intValue==-1);
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
	std::vector<std::string> Attribute::currentExpressionList;

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
	
	void Attribute::setExpressionList(std::vector<std::string> exprList)
	{
		this->expressionList=exprList;
	}
	
	std::vector<Arg>& Attribute::getArgList()
	{
		return this->argList;
	}
	
	std::vector<std::string> Attribute::getExpressionList()
	{
		return this->expressionList;
	}
	
	void Attribute::addParsedExpression(SgExpression* expr)
	{
		this->sgExpressionList.push_back(expr);
	}
	
	SgExpression* Attribute::getMapName()
	{
		return SageBuilder::buildStringVal(this->mapName);
	}
	
	SgExpression* Attribute::getMapNumber()
	{
		//check if parsing has been done
		if (this->sgExpressionList.size() != this->expressionList.size())
		{
			std::cerr << "Error : Parsing has not been done before accessing mapNumber" << std::endl;
			throw 0;
		}
		
		if (this->mapNumber.isInt())
			return SageBuilder::buildIntVal(this->mapNumber.getInt());
		else
		{
			//FIXME improve vector security
			int exprIndex = -1;
			for (unsigned int i=0; i<this->expressionList.size(); i++)
				if (this->expressionList[i] == this->mapNumber.getExpr())
					exprIndex = i;
			return this->sgExpressionList[exprIndex];
		}
	}
	
	int Attribute::argIndex(int arg)
	{
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			if (this->argList[i].argNumber==arg)
				return i;
		}
		std::cerr << "No information in pragma for argument n°" << arg << std::endl ;
		throw 0;
	}
	
	ArgType Attribute::argType(int argIndex)
	{
		if (this->argList[argIndex].argType!=_arg_unknown)
			return this->argList[argIndex].argType;
		std::cerr << "Missing argument type in pragma" << std::endl ;
		throw 0;
	}
	
	int Attribute::argDimension(int argIndex)
	{
		if (this->argList[argIndex].argRange.size()==0)
			return this->argList[argIndex].argSize.size();
		else if (this->argList[argIndex].argRange.size() == this->argList[argIndex].argSize.size())
		{
			int dimension = 0;
			for (unsigned int i=0; i<this->argList[argIndex].argSize.size(); i++)
			{
				//if range is [n] then it dosen't count toward dimension
				if (!(!this->argList[argIndex].argRange[i].first.isMinus1() and this->argList[argIndex].argRange[i].second.isMinus1()))
					dimension++;
			}
			return dimension;
		}
		std::cerr << "Size and range dimensions do not match in pragma" << std::endl ;
		throw 0;
	}
	
	//should not be called if arg is not a vector
	SgExpression* Attribute::argVectorSize(int argIndex)
	{
		
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
