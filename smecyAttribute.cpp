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
		return this->intExprToSgExpression(this->mapNumber);
	}
	
	SgExpression* Attribute::intExprToSgExpression(IntExpr ie)
	{
		//check if parsing has been done
		if (this->sgExpressionList.size() != this->expressionList.size())
		{
			std::cerr << "Error : Parsing has not been done before accessing IntExpr object" << std::endl;
			throw 0;
		}
		
		if (ie.isInt())
			return SageBuilder::buildIntVal(ie.getInt());
		else
		{
			//FIXME improve vector security
			int exprIndex = -1;
			for (unsigned int i=0; i<this->expressionList.size(); i++)
				if (this->expressionList[i] == ie.getExpr())
					exprIndex = i;
			return this->sgExpressionList[exprIndex];
		}
	}
	
	bool Attribute::checkAll()
	{
		int undocumentedArgs = 0;
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			//print a warning if exists arg with number >= number of Arg objects
			if (this->argList[i].argNumber > (int)this->argList.size())
				undocumentedArgs++;
			
			//presence of type information
			if (this->argList[i].argType == _arg_unknown)
			{
				std::cerr << "Error: missing type (in, ou, inout, unused) information for argument " << this->argList[i].argNumber
						<< " with size or range information." << std::endl;
				return false;
			}
			
			//check size information when range is used
			if (this->argList[i].argRange.size() > this->argList[i].argSize.size())
			{
				std::cerr << "Error: missing size information for argument " << this->argList[i].argNumber << " with range information." << std::endl;
				return false;
			}
			
			//contiguity of each arg in memory
			int level = 0; //[n]:0 [m:n]:1 []=2
			int currentLevel;
			for (unsigned int j=0; j<this->argList[i].argRange.size(); j++)
			{
				if (!this->argList[i].argRange[j].first.isMinus1() and this->argList[i].argRange[j].second.isMinus1()) //[n]
					currentLevel = 0;
				else if (!this->argList[i].argRange[j].first.isMinus1() and !this->argList[i].argRange[j].second.isMinus1()) //[m:n]
					currentLevel = 1;
				else if (this->argList[i].argRange[j].first.isMinus1() and this->argList[i].argRange[j].second.isMinus1()) //[]
					currentLevel = 2;
				else
				{
					std::cerr << "Error: corrupted range for argument " << this->argList[i].argNumber << " ." << std::endl;
					return false;
				}
				if (level > currentLevel)
				{
					std::cerr << "Error: data not contiguous in memory for argument " << this->argList[i].argNumber << " ." << std::endl;
					return false;
				}
				level = currentLevel;
			}
			
			//warning if actual dimension is >1
			int dimension = this->argDimension(i);
			if (dimension > 1)
				std::cerr << "Warning: argument is of dimension " << dimension << " for argument "
						<< this->argList[i].argNumber << "  but is used as a vector." << std::endl;
		}
		//this number is only a lower bound since we don't know the total number of arguments of the function
		if (undocumentedArgs)	
				std::cerr << "Warning: missing mapping information for at least " << undocumentedArgs << " different arguments." << std::endl;
		
		return true;
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
	int Attribute::argVectorAxis(int argIndex)
	{
		if (this->argList[argIndex].argSize.size()==1)
			return 0;
		else //we know range exists and has same size as size
		{
			for (unsigned int i=0; i<this->argList[argIndex].argSize.size(); i++)
			{
				if (!(!this->argList[argIndex].argRange[i].first.isMinus1() and this->argList[argIndex].argRange[i].second.isMinus1()))
					return i;
			}
		}
		std::cerr << "Arg is not a vector" << std::endl ;
		throw 0;
	}
	
	//should not be called if arg is not a vector
	SgExpression* Attribute::argVectorSize(int argIndex)
	{
		int axis = this->argVectorAxis(argIndex);
		//case where the size of the vector is given by argSize
		if ( (this->argList[argIndex].argRange.size()==0) or
				(this->argList[argIndex].argRange[axis].first.isMinus1() and this->argList[argIndex].argRange[axis].second.isMinus1()) )
			return this->intExprToSgExpression(this->argList[argIndex].argSize[axis]);
		else //case where the size of the vector is given by argRange
		{
			SgExpression* leftOperand = this->intExprToSgExpression(this->argList[argIndex].argRange[axis].second);
			SgExpression* rightOperand = this->intExprToSgExpression(this->argList[argIndex].argRange[axis].first);
			return SageBuilder::buildSubtractOp(leftOperand, rightOperand);
		}
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
