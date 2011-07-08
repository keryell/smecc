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
					
stream_loop_clause
					: STREAM_LOOP { Attribute::currentAttribute->addStreamLoop(); }
					;
					
stream_node_clause
					: STREAM_NODE '(' INTEGER ')' { Attribute::currentAttribute->addStreamNode($3); }
					;
					
if_clause
					: IF '(' int ')' { Attribute::currentAttribute->addIf(Attribute::currentIntExpr); }
					;

arg_clause
					: ARG '(' INTEGER ',' { Attribute::argNumber = $3; } arg_parameter_list ')'
					;
					
map_clause
					: MAP '(' ID closing_map_clause { 
						Attribute::currentAttribute->addMap($3,Attribute::currentIntExpr);
						}
					;
					
closing_map_clause
					: ')'
					| ',' int ')'
					;
					
arg_parameter_list
					: arg_parameter
					| arg_parameter ',' arg_parameter_list
					;
					
arg_parameter		
					: IN { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_in); }
					| OUT { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_out); }
					| INOUT { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_inout); }
					| UNUSED { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_unused); }
					| size
					| range
					;
					
size
					: '[' int ']' { Attribute::argSize.clear();
						Attribute::argSize.push_back(Attribute::currentIntExpr);
						} size_list
					;

size_list
					: /*empty*/ { Attribute::currentAttribute->addArg(Attribute::argNumber,Attribute::argSize);}
					| '[' int ']' { Attribute::argSize.push_back(Attribute::currentIntExpr); } size_list
					;

//here we have to trigger expression mode early otherwise first token of 'int' will not be lexed correctly
// FIXME the way expression mode is triggered should be improved to avoid this sort of problem
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

expr
					: /*empty*/
					| INTEGER { Attribute::expr << $1; } expr
					| EXPR_THING { Attribute::expr << $1; } expr
					| '(' { Attribute::expr << '('; } vexpr ')' { Attribute::expr << ')'; } expr
					| '[' { Attribute::expr << '['; } cexpr ']' { Attribute::expr << ']'; } expr
					;
					
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

int _yyparse()
{
	return yyparse();
}
