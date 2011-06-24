#ifndef SMECY_ATTRIBUTE_H
#define SMECY_ATTRIBUTE_H

#include "public.h"

//=================================================================================
// Defines attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//=================================================================================

namespace smecy
{
	enum ArgType
	{
		_arg_in,
		_arg_out,
		_arg_inout,
		_arg_unused,
		_arg_unknown
	};
	
	//can be either an int or an expression
	//for internal use
	class IntExpr
	{
	protected:
		bool isIntBool;
		int intValue;
		std::string exprValue;
	public:
		IntExpr(int intValue);
		IntExpr(std::string exprValue="");
	
		bool isExpr();
		bool isInt();
		int getInt();
		std::string getExpr();
		
		bool isMinus1();
	};

	//individual smecy clause
	class Arg
	{
	public:	//for internal use only
		int argNumber;
		ArgType argType;
		std::vector<IntExpr> argSize;
		std::vector<std::pair<IntExpr,IntExpr> > argRange;

		Arg();
		void print();
	};

	//all smecy clauses attached to one node
	class Attribute: public AstAttribute
	{
	protected:
		std::string mapName;
		IntExpr mapNumber;
		std::vector<Arg> argList;
		std::vector<std::string> expressionList;
		std::vector<SgExpression*> sgExpressionList;
	public:
		//methods needed the create the attribute
		void addArg(int argNumber, ArgType argType);
		void addArg(int argNumber, std::vector<IntExpr> argSize);
		void addArg(int argNumber, std::vector<std::pair<IntExpr,IntExpr> > argRange);
		void print();
		Attribute(std::string mapName, IntExpr mapNumber);
		void setExpressionList(std::vector<std::string> exprList);
		
		//FIXME remove this if possible
		std::vector<Arg>& getArgList();
		
		//expression-related methods
		void addParsedExpression(SgExpression* expr);
		std::vector<std::string> getExpressionList();
		
		//high-level get methods to get Sg objects directly
		SgExpression* getMapName();
		SgExpression* getMapNumber();
		
		//information about the args
		int argIndex(int arg); //returns index of arg in argList fails otherwise
		ArgType argType(int argIndex);
		int argDimension(int argIndex);	//return effective dimension, taking range into account
		SgExpression* argVectorSize(int argIndex); //TODO
	
		//static attributes needed for parsing
		static Attribute *currentAttribute ;
		static std::vector<IntExpr> argSize;
		static std::vector<std::pair<IntExpr,IntExpr> > argRange;
		static std::pair<IntExpr,IntExpr> currentPair;
		static int argNumber;
		static int isExprMode;
		static std::stringstream expr;
		static IntExpr currentIntExpr;
		static std::vector<std::string> currentExpressionList;
	};
}//namespace smecy

std::ostream& operator<<(std::ostream& os, smecy::IntExpr& ie);

#endif //SMECY_ATTRIBUTE_H
