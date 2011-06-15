#ifndef SMECY_ATTRIBUTE_H
#define SMECY_ATTRIBUTE_H

#include "public.h"

//=================================================================================
// Defines attributes destined to be attached to SMECY pragma nodes of the ROSE AST
//=================================================================================

enum smecyClauseType
{
	smecy_map,
	smecy_arg_type,
	smecy_arg_size,
	smecy_arg_range
};

enum smecyArgType
{
	smecy_arg_in,
	smecy_arg_out,
	smecy_arg_inout,
	smecy_arg_unused
};

//individual smecy clause
class smecyClause
{
protected:
	smecyClauseType type;
	//TODO could be made shorter with multiple child classes
	std::string accelerator;
	int unitNumber;
	int argNumber;
	smecyArgType argType;
	std::vector<int> argSize;
	std::vector<std::pair<int,int> > argRange;
public:
	//constructors
	smecyClause(std::string accelerator, int unitNumber=-1);
	smecyClause(int argNumber, smecyArgType argType);
	smecyClause(int argNumber, std::vector<int> argSize);
	smecyClause(int argNumber, std::vector<std::pair<int,int> > argRange);

	void print();
};

//all smecy clauses attached to one node
class smecyAttribute: public AstAttribute
{
protected:
	std::vector<smecyClause> clauseList;
public:
	void addSmecyClause(smecyClause clause);
	//TODO need for a method that returns the original pragma string ?
	void print();
	
	//static attributes needed for parsing
	static smecyAttribute *attributeBeingBuilt ;
	static std::vector<int> argSize;
	static std::vector<std::pair<int,int> > argRange;
	static int argNumber;
};

#endif //SMECY_ATTRIBUTE_H
