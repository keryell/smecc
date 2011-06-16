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

	//individual smecy clause
	class Clause
	{
	protected:
		ClauseType type;
		//TODO could be made shorter with multiple child classes
		std::string accelerator;
		int unitNumber;
		int argNumber;
		ArgType argType;
		std::vector<int> argSize;
		std::vector<std::pair<int,int> > argRange;
	public:
		//constructors
		Clause(std::string accelerator, int unitNumber=-1);
		Clause(int argNumber, ArgType argType);
		Clause(int argNumber, std::vector<int> argSize);
		Clause(int argNumber, std::vector<std::pair<int,int> > argRange);

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
		static Attribute *attributeBeingBuilt ;
		static std::vector<int> argSize;
		static std::vector<std::pair<int,int> > argRange;
		static int argNumber;
		static int isExprMode;
		static std::stringstream expr;
	};
}//namespace smecy

#endif //SMECY_ATTRIBUTE_H
