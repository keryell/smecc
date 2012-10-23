#ifndef SMECY_ATTRIBUTE_H
#define SMECY_ATTRIBUTE_H

#include "public.hpp"

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

	//can be either an int or an expression or a SgExpression
	//for internal use
	class IntExpr
	{
	protected:
		//booleans to know the type
		bool isIntBool;
		bool isSgBool;

		//actual values
		int intValue;
		std::string exprValue;
		SgExpression* sgExpr;
	public:
		//one constructor for each type
		IntExpr(int intValue);
		IntExpr(SgExpression* sgExpr);
		IntExpr(std::string exprValue="");

		//methods to know the type
		bool isExpr();
		bool isSgExpr();
		bool isInt();

		//get method when type is know (returns 0/NULL/"" when wrong type)
		int getInt();
		std::string getExpr();
		SgExpression* getSgExpr();

		//returns true if the value is -1
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
		SgNode* parent;
		std::string mapName; //name of the accelerator to map to
		IntExpr mapNumber; //number of said accelerator
		std::vector<Arg> argList; //list of arguments
		std::vector<std::string> expressionList; //FIXME refactor to keep only a list of IntExpr
		std::vector<SgExpression*> sgExpressionList;
		IntExpr condition; //condition if if clause
		int streamLoop; //number of stream loop or -1 if not a stream loop
        int stage; //stage number or -1 if nor a stage in a pipelined stream loop
        int label; //number of label or -1 if no label

		//private methods
		int argIndex(int arg); //returns index of arg in argList -1 otherwise

	public: //TODO move methods to protected
		void print();
		Attribute(SgNode* parent=NULL); //TODO add new constructors

		//adding clauses
		void addMap(std::string mapName, IntExpr mapNumber);
		void addArg(int argNumber, ArgType argType);
		void addArg(int argNumber, std::vector<IntExpr> argSize);
		void addArg(int argNumber, std::vector<std::pair<IntExpr,IntExpr> > argRange);
		void addIf(IntExpr condition);
		void addStreamLoop();
		void addStage();
        void addLabel(int number);
		static int streamLoopTotal;
        static int currentStreamStage;

		//get and set methods
		void addParsedExpression(SgExpression* expr);
		std::vector<std::string> getExpressionList();
		void setExpressionList(std::vector<std::string> exprList);
		SgExpression* getMapName(SgScopeStatement* scope);
		SgExpression* getMapNumber();
		SgExpression* intExprToSgExpression(IntExpr ie);
		SgExpression* argSizeExp(int arg);
		SgExpression* getIf();

		//methods to know the kind of pragma
		int getStreamLoop();
		int getLabel();
        int getStage();
		bool hasMapClause();
		bool hasArgClause();
		bool hasIfClause();

		//top level method to check correctness of pragma information
		bool checkAll();

		//information about the args
		int getArgNumber();
		ArgType argType(int arg);
		int argDimension(int arg);	//returns effective dimension, taking range into account
		std::vector<IntExpr>& getSize(int arg); //FIXME FIXME FIXME

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
		static SgNode* currentParent;
	};
}//namespace smecy

std::ostream& operator<<(std::ostream& os, smecy::IntExpr& ie);

#endif //SMECY_ATTRIBUTE_H
