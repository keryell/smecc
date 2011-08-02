/* SMECY pragma parser */

%{
#include "public.h"
#include "smecyAttribute.h"

using namespace smecy;

int yylex(void);
void yyerror(char *); 
int _yyparse();
%}

%union
{
	int intType;
	const char *stringType;
}

%token SMECY MAP ARG '[' ']' '/' '(' ')' ':' ',' IN OUT INOUT UNUSED NOPAR STREAM_LOOP STREAM_NODE IF
%token <intType> INTEGER
%token <stringType> ID EXPR_THING

%start smecy_directive

%%
/* a smecy directive is a list of clauses 
the static attribute object that will be used
to store the result of the parsing is initialized
a list meant to contain all C expressions contained in
the directive is also initilaized*/
smecy_directive
					: SMECY { Attribute::currentExpressionList.clear(); Attribute::currentAttribute = new Attribute(Attribute::currentParent); }
					  clause_list { Attribute::currentAttribute->setExpressionList(Attribute::currentExpressionList) }
					;
					
clause_list
					: /*empty*/
					| map_clause clause_list
					| arg_clause clause_list
					| if_clause clause_list
					| stream_loop_clause clause_list
					| stream_node_clause clause_list
					;

/*From there, we use the various methods of Attribute objects
(see smecyAttribute.h) to add the information regarding
the different types of clause
*/
			
stream_loop_clause
					: STREAM_LOOP { Attribute::currentAttribute->addStreamLoop(); }
					;
					
stream_node_clause
					: STREAM_NODE '(' INTEGER ')' { Attribute::currentAttribute->addStreamNode($3); }
					;

/* RK: normalement il suffit d'utiliser $$ = ... pour passer des
   paramètres entre règles plutôt que de passer par des variables globales,
   des arg_ranges.push_back(), Attribute::currentIntExpr et tout et tout...
   Cf la doc de bison :-)
   J'ai peur si Rose est programmé comme ça... :-/
*/
if_clause
					: IF '(' int ')' { Attribute::currentAttribute->addIf(Attribute::currentIntExpr); }
					/* some computation is hidden in int*/
					;

arg_clause
					: ARG '(' INTEGER ',' { Attribute::argNumber = $3; } arg_parameter_list ')'
					/* some computation is hidden in arg_parameter_list*/
					;
					
map_clause
					: MAP '(' ID closing_map_clause { 
						Attribute::currentAttribute->addMap($3,Attribute::currentIntExpr);
						}
					;
					
closing_map_clause
					: ')'
					| ',' int ')'
					/* some computation is hidden in int*/
					;
					
arg_parameter_list
					: arg_parameter
					| arg_parameter ',' arg_parameter_list
					/* some computation is hidden in arg_parameter*/
					;
					
arg_parameter		
					: IN { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_in); }
					| OUT { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_out); }
					| INOUT { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_inout); }
					| UNUSED { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_unused); }
					| size
					| range
					/* some computation is hidden in size and range*/
					;
					
size
					: '[' int ']' { Attribute::argSize.clear();
						Attribute::argSize.push_back(Attribute::currentIntExpr);
						} size_list
					/* some computation is hidden in int*/
					;

size_list
					: /*empty*/ { Attribute::currentAttribute->addArg(Attribute::argNumber,Attribute::argSize);}
					| '[' int ']' { Attribute::argSize.push_back(Attribute::currentIntExpr); } size_list
					/* some computation is hidden in int*/
					;

// RK: ah mais quelle horreur !
// Voilà pourquoi cela m'a échappé ! Normalement on utilise les "Start Conditions" pour faire ça. Cf. la doc de flex, section 10.

//here we have to trigger expression mode early otherwise first token of 'int' will not be lexed correctly
// FIXME the way expression mode is triggered should be improved to avoid this sort of problem
/* range is divided in several parts to cover the different cases ([], [n] and [n:m])
a range is made of pairs of intExpr with the following correspondance :
[] -> (-1,-1)
[n] -> (n,-1)
[n:m] -> (n,m)
*/
range
					: '/' '[' { Attribute::argRange.clear(); Attribute::isExprMode=1; } range_begin
					;

range_begin
					: int { Attribute::currentPair.first = Attribute::currentIntExpr; } range_mid
					| ']' { 
					  Attribute::argRange.push_back(std::pair<IntExpr,IntExpr>(IntExpr(-1),IntExpr(-1)));
					  Attribute::isExprMode=0; } 
					  range_open
					;
					
range_mid
					: ':' int { Attribute::currentPair.second = Attribute::currentIntExpr;
					  Attribute::argRange.push_back(Attribute::currentPair); } 
					  ']' range_open
					| ']' { Attribute::currentPair.second = IntExpr(-1); 
					  Attribute::argRange.push_back(Attribute::currentPair); } 
					  range_open
					;
					
range_open
					: /*empty*/ { Attribute::currentAttribute->addArg(Attribute::argNumber,Attribute::argRange);}
					| '[' { Attribute::isExprMode=1; } range_begin
					;

//isExprMode=1 means we enter "expression mode"
//in this mode the lexer will return EXPR_THING for everything except ( ) [ } ,
//int is a generic C expression (can also be a boolean)
//parenthesis are counted so as to make sure the expression does not
//include parts of the directive
int
					: { Attribute::isExprMode=1; Attribute::expr.str(""); } 
					  int_expr 
					  { Attribute::isExprMode=0; }
					;
					
int_expr
					: INTEGER { Attribute::currentIntExpr = IntExpr($1); }
					| INTEGER { Attribute::expr << $1; } EXPR_THING { Attribute::expr << $3; } 
					  expr { Attribute::currentIntExpr = IntExpr(Attribute::expr.str()); 
					  Attribute::currentExpressionList.push_back(Attribute::expr.str()); }
					| EXPR_THING { Attribute::expr << $1; } expr { Attribute::currentIntExpr = IntExpr(Attribute::expr.str());
					  Attribute::currentExpressionList.push_back(Attribute::expr.str()); }
					;

// RK: du coup tout ceci me semble bien compliqué...
expr
					: /*empty*/
					| INTEGER { Attribute::expr << $1; } expr
					| EXPR_THING { Attribute::expr << $1; } expr
					| '(' { Attribute::expr << '('; } vexpr ')' { Attribute::expr << ')'; } expr
					| '[' { Attribute::expr << '['; } cexpr ']' { Attribute::expr << ']'; } expr
					;

//we need vexpr (expressions where commas are allowed) and cexpr (same with colons)
//since we want to allow commas and colons in certain parts of expressions
//(but not anywhere in the expressions since commas and colons are used
//to separate the expression from the pragma)
vexpr
					: expr
					| expr ',' { Attribute::expr << ','; } vexpr
					;

cexpr
					: expr
					| expr ':' { Attribute::expr << ':'; } vexpr
					;
%%

void yyerror(char *s)
{
	std::cerr << s << std::endl;
}

/* function yyparse needs to be wrapped so as to
be used in another file (see smecyLexer.ll
*/
int _yyparse()
{
	return yyparse();
}
