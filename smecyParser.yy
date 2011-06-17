/* SMECY pragma parser */

%{
#include "public.h"
#include "smecyAttribute.h"

int yylex(void);
void yyerror(char *); 
int _yyparse();
%}

%union
{
	int intType;
	const char *stringType;
}

%token SMECY MAP ARG '[' ']' '/' '(' ')' ':' ',' IN OUT INOUT UNUSED NOPAR
%token <intType> INTEGER
%token <stringType> ID EXPR_THING

%start smecy_directive

%%
smecy_directive
					: SMECY { smecy::Attribute::currentAttribute=new smecy::Attribute(); } arg_clause arg_clause_list	//we do not allow empty directives
					| SMECY { smecy::Attribute::currentAttribute=new smecy::Attribute(); } map_clause arg_clause_list
					;

//arg_clause puts the arg number on top of the stack to be passed down
arg_clause
					: ARG '(' int ',' { smecy::Attribute::argNumber = smecy::Attribute::currentIntExpr; } arg_parameter_list ')'
					;
					
arg_clause_list
					: // empty 
					| arg_clause arg_clause_list
					;
					
map_clause
					: MAP '(' ID closing_map_clause { 
						smecy::Attribute::currentAttribute->addClause(smecy::Clause($3,smecy::Attribute::currentIntExpr));
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
					: IN { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_in)); }
					| OUT { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_out)); }
					| INOUT { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_inout)); }
					| UNUSED { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_unused)); }
					| size
					| range
					;
					
size
					: '[' int ']' { smecy::Attribute::argSize.clear();
						smecy::Attribute::argSize.push_back(smecy::Attribute::currentIntExpr);
						} size_list
					;

size_list
					: /*empty*/ { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::Attribute::argSize));}
					| '[' int ']' { smecy::Attribute::argSize.push_back(smecy::Attribute::currentIntExpr); } size_list
					;

range
					: '/' '[' { smecy::Attribute::argRange.clear(); smecy::Attribute::isExprMode=1; } range_begin
					;

range_begin
					: int { smecy::Attribute::currentPair.first = smecy::Attribute::currentIntExpr; } range_mid
					| ']' { 
					  smecy::Attribute::argRange.push_back(std::pair<smecy::intExpr,smecy::intExpr>(smecy::intExpr(-1),smecy::intExpr(-1)));
					  smecy::Attribute::isExprMode=0; } 
					  range_open
					;
					
range_mid
					: ':' int { smecy::Attribute::currentPair.second = smecy::Attribute::currentIntExpr;
					  smecy::Attribute::argRange.push_back(smecy::Attribute::currentPair); } 
					  ']' range_open
					| ']' { smecy::Attribute::currentPair.second = smecy::intExpr(-1); 
					  smecy::Attribute::argRange.push_back(smecy::Attribute::currentPair); } 
					  range_open
					;
					
range_open
					: /*empty*/ { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::Attribute::argRange));}
					| '[' { smecy::Attribute::isExprMode=1; } range_begin
					;

int
					: { smecy::Attribute::isExprMode=1; smecy::Attribute::expr.str(""); } 
					  int_expr 
					  { smecy::Attribute::isExprMode=0; }
					;
					
int_expr
					: INTEGER { smecy::Attribute::currentIntExpr = smecy::intExpr($1); }
					| EXPR_THING { smecy::Attribute::expr << $1; } expr { smecy::Attribute::currentIntExpr = smecy::intExpr(smecy::Attribute::expr.str()); }
					;

expr
					: /*empty*/
					| INTEGER { smecy::Attribute::expr << $1; } expr
					| EXPR_THING { smecy::Attribute::expr << $1; } expr
					| '(' { smecy::Attribute::expr << '('; } vexpr ')' { smecy::Attribute::expr << ')'; } expr
					| '[' { smecy::Attribute::expr << '['; } cexpr ']' { smecy::Attribute::expr << ']'; } expr
					;
					
vexpr
					: expr
					| expr ',' { smecy::Attribute::expr << ','; } vexpr
					;

cexpr
					: expr
					| expr ':' { smecy::Attribute::expr << ':'; } vexpr
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
