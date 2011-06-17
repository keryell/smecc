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

%type <intType> closing_map_clause int int_expr

%start smecy_directive

%%
smecy_directive
					: SMECY { smecy::Attribute::currentAttribute=new smecy::Attribute(); } arg_clause arg_clause_list	//we do not allow empty directives
					| SMECY { smecy::Attribute::currentAttribute=new smecy::Attribute(); } map_clause arg_clause_list
					;

//arg_clause puts the arg number on top of the stack to be passed down
arg_clause
					: ARG '(' int ',' { smecy::Attribute::argNumber = $3; } arg_parameter_list ')'
					;
					
arg_clause_list
					: // empty 
					| arg_clause arg_clause_list
					;
					
map_clause
					: MAP '(' ID closing_map_clause { 
						smecy::Attribute::currentAttribute->addClause(smecy::Clause($3,$4));
						}
					;
					
closing_map_clause
					: ')' { $$ = -1; }
					| ',' int ')' { $$ = $2; }	//FIXME type de retour de int
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
						smecy::Attribute::argSize.push_back($2);
						} size_list
					;

size_list
					: /*empty*/ { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::Attribute::argSize));}
					| '[' int ']' { smecy::Attribute::argSize.push_back($2); } size_list
					;

range
					: '/' '[' { smecy::Attribute::argRange.clear(); smecy::Attribute::isExprMode=1; } range_begin
					;

range_begin
					: int { smecy::Attribute::currentPair.first = $1; } range_mid
					| ']' { smecy::Attribute::argRange.push_back(std::pair<int,int>(-1,-1)); smecy::Attribute::isExprMode=0; } range_open
					;
					
range_mid
					: ':' int { smecy::Attribute::currentPair.second = $2; smecy::Attribute::argRange.push_back(smecy::Attribute::currentPair); } ']' range_open
					| ']' { smecy::Attribute::currentPair.second = -1; smecy::Attribute::argRange.push_back(smecy::Attribute::currentPair); } range_open
					;
					
range_open
					: /*empty*/ { smecy::Attribute::currentAttribute->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::Attribute::argRange));}
					| '[' { smecy::Attribute::isExprMode=1; } range_begin
					;

int
					: { smecy::Attribute::isExprMode=1; smecy::Attribute::expr.str(""); } 
					  int_expr 
					  { smecy::Attribute::isExprMode=0; $$ = $2; }
					;
					
int_expr
					: INTEGER { $$ = $1; }
					| EXPR_THING { smecy::Attribute::expr << $1; } expr { $$ = -1; }
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
