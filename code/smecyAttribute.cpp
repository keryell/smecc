#ifndef SMECY_ATTRIBUTE_CPP
#define SMECY_ATTRIBUTE_CPP

#include "smecyAttribute.hpp"

//====================================================================================
// Implements attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//====================================================================================

namespace smecy
{
//==========================================================================//
// IntExpt
//==========================================================================//
	//constructors
	IntExpr::IntExpr(int intValue) : intValue(intValue), exprValue(""), sgExpr(NULL)
	{
		this->isIntBool = true;
		this->isSgBool = false;
	}

	IntExpr::IntExpr(std::string exprValue) : intValue(0), exprValue(exprValue), sgExpr(NULL)
	{
		this->isIntBool = false;
		this->isSgBool = false;
	}

	IntExpr::IntExpr(SgExpression* sgExpr) : intValue(0), exprValue(""), sgExpr(sgExpr)
	{
		this->isIntBool = false;
		this->isSgBool = true;
	}

	//methods to know the type
	bool IntExpr::isExpr()
	{
		return (!this->isIntBool) and (!this->isSgBool);
	}

	bool IntExpr::isSgExpr()
	{
		return this->isSgBool;
	}

	bool IntExpr::isInt()
	{
		return this->isIntBool;
	}

	//get methods
	int IntExpr::getInt()
	{
		if (this->isIntBool)
			return this->intValue;
		else
			return 0;
	}

	SgExpression* IntExpr::getSgExpr()
	{
		if (this->isSgBool)
			return this->sgExpr;
		else
			return NULL;
	}

	std::string IntExpr::getExpr()
	{
		if (this->isIntBool)
			return "";
		else
			return this->exprValue;
	}

	//returns true if is -1
	bool IntExpr::isMinus1()
	{
		return (this->isIntBool and this->intValue==-1);
	}

//==========================================================================//
// Attribute
	//initializes all fields to a value that means the field is uninitialized
	Attribute::Attribute(SgNode* parent): parent(parent), mapName(""), mapCoordinates(std::vector<IntExpr>{}), condition(IntExpr(-1)), streamLoop(-1), stage(-1), label(-1)
	{
	}

  // All the static attributes used in parsing
  Attribute* Attribute::currentAttribute ;
  std::vector<IntExpr> Attribute::args;
  std::vector<std::pair<IntExpr,IntExpr> > Attribute::argRange;
  int Attribute::argNumber;
  std::stringstream Attribute::expr;
  std::pair<IntExpr,IntExpr> Attribute::currentPair;
  IntExpr Attribute::currentIntExpr;
  std::vector<std::string> Attribute::currentExpressionList;
  SgNode* Attribute::currentParent = NULL;

  /* static attribute used in counting the stream loops (smecy
     lowering part, see smecyTranslation) */
  static int streamLoopTotal = 0;
  static int currentStreamStage;

	//methods to add clauses
	void Attribute::addMap(std::string mapName, std::vector<IntExpr> coordinates)
	{
		this->mapName = mapName;
		this->mapCoordinates = coordinates;
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

	void Attribute::addIf(IntExpr condition)
	{
		this->condition = condition;
	}

	//uses the static couonter streamLoopTotal
	void Attribute::addStreamLoop()
	{
		this->streamLoop = streamLoopTotal;
		streamLoopTotal++;
		// Reset the current stage numbering
		currentStreamStage = 0;
	}

	void Attribute::addStage()
	{
		this->stage = currentStreamStage++;
	}

	void Attribute::addLabel(int number)
	    {
	        this->label = number;
	    }

	    //get and set methods

	void Attribute::setExpressionList(std::vector<std::string> exprList)
	{
		this->expressionList=exprList;
	}

	std::vector<std::string> Attribute::getExpressionList()
	{
		return this->expressionList;
	}

	void Attribute::addParsedExpression(SgExpression* expr)
	{
		this->sgExpressionList.push_back(expr);
	}

	SgExpression* Attribute::getMapName(SgScopeStatement* scope)
	{
		//we use opaque var ref since
		return SageBuilder::buildOpaqueVarRefExp(this->mapName, scope);
	}

	std::vector<SgExpression*> Attribute::getMapCoordinates() {
	  std::vector<SgExpression*> c;
	  // Parse all the integer expressions of the accelerator coordinates
	  for (auto e: this->mapCoordinates)
	    c.push_back(intExprToSgExpression(e));
	  return c;
	}

	//transforms a intExpr into a SgExpression regardless of the actual type stored in the intexpr
	//parsing of expressions must have been done (see smecyTranslation)
	SgExpression* Attribute::intExprToSgExpression(IntExpr ie)
	{
		//check if parsing has been done
		if (this->sgExpressionList.size() != this->expressionList.size())
		{
			std::cerr << debugInfo(this->parent) << "error: Parsing has not been done before accessing IntExpr object" << std::endl;
			throw 0;
		}
		if (ie.isSgExpr())
			return ie.getSgExpr();
		else if (ie.isInt())
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

	//returns the expression that gives the size of an argument
	SgExpression* Attribute::argSizeExp(int arg)
	{
		int index = this->argIndex(arg);
		if (this->argList[index].argSize.size() == 0) //scalar
			return SageBuilder::buildIntVal(1);
		else if (this->argList[index].argRange.size() == 0) //no range, size is product of argSize elements
		{
			SgExpression* result = this->intExprToSgExpression(this->argList[index].argSize[0]);
			for (unsigned int i=1; i<this->argList[index].argSize.size(); i++)
				result = SageBuilder::buildMultiplyOp(result, this->intExprToSgExpression(this->argList[index].argSize[i]));
			return result;
		}
		else if (this->argList[index].argRange.size() == this->argList[index].argSize.size()) //argRange is present, size depends on range form
		{
			SgExpression* result = NULL;
			SgExpression* partialResult = NULL;
			for (unsigned int i=0; i<this->argList[index].argSize.size(); i++)
			{
				if (this->argList[index].argRange[i].first.isMinus1() and this->argList[index].argRange[i].second.isMinus1()) //[]
					partialResult = this->intExprToSgExpression(this->argList[index].argSize[i]);
				else if (!this->argList[index].argRange[i].first.isMinus1() and this->argList[index].argRange[i].second.isMinus1()) //[n]
					{ /* result = result * 1 */ }
				else if (!this->argList[index].argRange[i].first.isMinus1() and !this->argList[index].argRange[i].second.isMinus1()) //[n:m]
					partialResult = SageBuilder::buildSubtractOp(
							SageBuilder::buildAddOp(
								this->intExprToSgExpression(this->argList[index].argRange[i].second),
								SageBuilder::buildIntVal(1)),
							this->intExprToSgExpression(this->argList[index].argRange[i].first)
							); //result = result * (m-n+1)
				else
				{
					std::cerr << debugInfo(this->parent) << "error: corrupted range for argument " << arg << " ." << std::endl;
					throw 0;
				}
				if (!result) // to avoid useless 1* expression
					result = partialResult;
				else
					result = SageBuilder::buildMultiplyOp(result, partialResult);
			}
			if (!result) //case where range is /[m][n]...[p]
				return SageBuilder::buildIntVal(1);
			else
				return result;
		}
		else
		{
			std::cerr << debugInfo(this->parent) << "error: range and size information do not match for argument " << arg << "." << std::endl;
			throw 0;
		}
	}

	SgExpression* Attribute::getIf()
	{
		if (this->condition.isMinus1())
			return NULL;
		else
			return this->intExprToSgExpression(this->condition);
	}

	int Attribute::getStreamLoop()
	{
		return this->streamLoop;
	}

    int Attribute::getStage()
    {
        return this->stage;
    }

	int Attribute::getLabel()
	{
		return this->label;
	}

	bool Attribute::hasMapClause()
	{
		return (this->mapName!="");
	}

	bool Attribute::hasArgClause()
	{
		return (this->argList.size()!=0);
	}

	bool Attribute::hasIfClause()
	{
		return (this->getIf()!=NULL);
	}

	/*proceeds to various checks on the attribute*/
	bool Attribute::checkAll()
	{
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			//presence of type information
			if (this->argList[i].argType == _arg_unknown)
			{
				std::cerr << debugInfo(this->parent) << "error: missing type (in, ou, inout, unused) information for argument " << this->argList[i].argNumber
						<< " with size or range information." << std::endl;
				return false;
			}

			//check size information when range is used
			if (this->argList[i].argRange.size() > this->argList[i].argSize.size())
			{
				std::cerr << debugInfo(this->parent) << "error: missing size information for argument "
						<< this->argList[i].argNumber << " with range information." << std::endl;
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
					std::cerr << debugInfo(this->parent) << "error: corrupted range for argument " << this->argList[i].argNumber << " ." << std::endl;
					return false;
				}
				if (level > currentLevel)
				{
					std::cerr << debugInfo(this->parent) << "error: data not contiguous in memory for argument "
							<< this->argList[i].argNumber << " ." << std::endl;
					return false;
				}
				level = currentLevel;
			}

			//warning if actual dimension is >1
			int dimension = this->argDimension(i);
			if (dimension > 1)
				std::cerr << debugInfo(this->parent) << "warning: argument " << this->argList[i].argNumber
						<< " is of dimension " <<dimension << " but is used as a vector." << std::endl;
		}

		return true;
	}

	//gets the index of an argument (in the Arg vector) from its index in the function's parameter list
	int Attribute::argIndex(int arg)
	{
		for (unsigned int i=0; i<this->argList.size(); i++)
		{
			if (this->argList[i].argNumber==arg)
				return i;
		}
		//std::cerr << debugInfo(this->parent) << "warning: no information in pragma for argument n°" << arg << std::endl ;
		return -1;
	}

	int Attribute::getArgNumber()
	{
		return this->argList.size();
	}

	ArgType Attribute::argType(int arg)
	{
		int argIndex = this->argIndex(arg);
		if (argIndex==-1)
			return _arg_in; //default is a scalar
		else if (this->argList[argIndex].argType!=_arg_unknown)
			return this->argList[argIndex].argType;
		else
		{
			std::cerr << debugInfo(this->parent) << "error: no type for non-scalar argument n°" << this->argList[argIndex].argNumber << std::endl ;
			throw 0;
		}
	}

	int Attribute::argDimension(int arg)
	{
		int argIndex = this->argIndex(arg);
		if (argIndex==-1)
			return 0; //default is a scalar
		else
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
			std::cerr << debugInfo(this->parent) << "error: Size and range dimensions do not match in pragma" << std::endl ;
			throw 0;
		}
	}

	std::vector<IntExpr>& Attribute::getSize(int arg)
	{
		int argIndex = this->argIndex(arg);
		return this->argList[argIndex].argSize;
	}

  void Attribute::print() {
    std::cout << "Attribute with " << this->argList.size() << " argument clauses." << std::endl;
    std::cout << "\tMapped to " << this->mapName<< " n°(";

    for (auto e: this->mapCoordinates)
      std::cout << e << ',';
    std::cout << ")." << std::endl;

    for (unsigned int i=0; i<this->argList.size(); i++) {
        std::cout << "\t" ;
        this->argList[i].print();
    }

    std::cout << "Parent = " << parent->unparseToString();

    std::cout << "expressionList:" << std::endl;
    for (auto e: this->expressionList)
      std::cout << e << std::endl;
    std::cout << ")." << std::endl;

    std::cout << "sgExpressionList:" << std::endl;
    for (auto e: this->sgExpressionList)
      std::cout << e->unparseToString() << std::endl;
    std::cout << ")." << std::endl;
  }
        //    IntExpr condition; //condition if if clause
        //    int streamLoop; //number of stream loop or -1 if not a stream loop
        //    int stage; //stage number or -1 if nor a stage in a pipelined stream loop
        //    int label; //number of label or -1 if no label

//==========================================================================//
// Arg
	Arg::Arg() : argNumber(-1), argType(_arg_unknown)
	{
	}

	void Arg::print()
	{
		std::cout << "arg(" << this->argNumber;
		if (this->argType != _arg_unknown)
		{
			std::cout << ", ";
			switch (this->argType)
			{
				case (_arg_in):
					std::cout << "in"; break;
				case (_arg_out):
					std::cout << "out"; break;
				case (_arg_inout):
					std::cout << "inout"; break;
				default:
					std::cerr << "error: invalid arg clause." << std::endl; throw 0;
			}
		}
		if (this->argSize.size() != 0)
		{
			std::cout << ", ";
			for (unsigned int i=0; i<this->argSize.size(); i++)
				std::cout << "[" << this->argSize[i] << "]";
		}
		if (this->argRange.size() != 0)
		{
			std::cout << ", /";
			for (unsigned int i=0; i<this->argRange.size(); i++)
			{
				if (this->argRange[i].first.isMinus1() and this->argRange[i].second.isMinus1())
					std::cout << "[]";
				else if (!this->argRange[i].first.isMinus1() and this->argRange[i].second.isMinus1())
					std::cout << "[" << this->argRange[i].first << "]";
				else if (!this->argRange[i].first.isMinus1() and !this->argRange[i].second.isMinus1())
					std::cout << "[" << this->argRange[i].first << ":" << this->argRange[i].second << "]";
			}
		}

		std::cout << ")" << std::endl;
	}
} // namespace smecy

std::ostream& operator<<(std::ostream& os, smecy::IntExpr& ie)
{
	if (ie.isInt())
		os << ie.getInt();
	else if (!ie.isSgExpr())
		os << ie.getExpr();
	else
		os << ie.getSgExpr()->unparseToString();
	return os;
}

#endif //SMECY_ATTRIBUTE_CPP
