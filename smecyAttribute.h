#ifndef SMECY_ATTRIBUTE_H
#define SMECY_ATTRIBUTE_H

#include "public.h"

//=================================================================================
// Defines attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//=================================================================================

namespace smecy
{
	enum ClauseType
	{
		_map,
		_arg_type,
		_arg_size,
		_arg_range
	};

	enum ArgType
	{
		_arg_in,
		_arg_out,
		_arg_inout,
		_arg_unused
	};
	
	//can be either an int or an expression
	//for internal use
	class intExpr
	{
	protected:
		bool isIntBool;
		int intValue;
		std::string exprValue;
	public:
		intExpr(int intValue);
		intExpr(std::string exprValue="");
	
		bool isExpr();
		bool isInt();
		int getInt();
		std::string getExpr();
	};

	//individual smecy clause
	class Clause
	{
	protected:
		ClauseType type;
		//TODO could be made shorter with multiple child classes
		std::string accelerator;
		intExpr unitNumber;
		intExpr argNumber;
		ArgType argType;
		std::vector<intExpr> argSize;
		std::vector<std::pair<intExpr,intExpr> > argRange;
	public:
		//constructors
		Clause(std::string accelerator, intExpr unitNumber=intExpr(-1));
		Clause(intExpr argNumber, ArgType argType);
		Clause(intExpr argNumber, std::vector<intExpr> argSize);
		Clause(intExpr argNumber, std::vector<std::pair<intExpr,intExpr> > argRange);

		void print();
	};

	//all smecy clauses attached to one node
	class Attribute: public AstAttribute
	{
	protected:
		std::vector<Clause> clauseList;
	public:
		void addClause(Clause clause);
		//TODO need for a method that returns the original pragma string ?
		void print();
	
		//static attributes needed for parsing
		static Attribute *currentAttribute ;
		static std::vector<intExpr> argSize;
		static std::vector<std::pair<intExpr,intExpr> > argRange;
		static std::pair<intExpr,intExpr> currentPair;
		static intExpr argNumber;
		static int isExprMode;
		static std::stringstream expr;
		static intExpr currentIntExpr;
	};
}//namespace smecy

std::ostream& operator<<(std::ostream& os, smecy::intExpr& ie);

#endif //SMECY_ATTRIBUTE_H
