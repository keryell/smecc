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
%token <intType> INTEGER;
%token <stringType> ID EXPR_THING;

%type <intType> closing_map_clause int

%nonassoc INTEGER

%start smecy_directive;

%%
smecy_directive
					: SMECY { smecy::Attribute::attributeBeingBuilt=new smecy::Attribute(); } arg_clause arg_clause_list	//we do not allow empty directives
					| SMECY { smecy::Attribute::attributeBeingBuilt=new smecy::Attribute(); } map_clause arg_clause_list
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
						smecy::Attribute::attributeBeingBuilt->addClause(smecy::Clause($3,$4));
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
					: IN { smecy::Attribute::attributeBeingBuilt->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_in)); }
					| OUT { smecy::Attribute::attributeBeingBuilt->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_out)); }
					| INOUT { smecy::Attribute::attributeBeingBuilt->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_inout)); }
					| UNUSED { smecy::Attribute::attributeBeingBuilt->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::_arg_unused)); }
					| size
					| range
					;
					
size
					: '[' int ']' { smecy::Attribute::argSize.clear();
						smecy::Attribute::argSize.push_back($2);
						} size_list
					; //FIXME allow expressions?

size_list
					: /*empty*/ { smecy::Attribute::attributeBeingBuilt->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::Attribute::argSize));}
					| '[' int ']' { smecy::Attribute::argSize.push_back($2); } size_list
					;

range
					: '/' '[' int ':' int ']' { smecy::Attribute::argRange.clear();
						smecy::Attribute::argRange.push_back(std::pair<int,int>($3,$5));
						} range_list	//TODO add possibility for empty or "single" ranges
					;
					
range_list
					: /*empty*/ { smecy::Attribute::attributeBeingBuilt->addClause(smecy::Clause(smecy::Attribute::argNumber,smecy::Attribute::argRange));}
					| '[' int ':' int ']' { smecy::Attribute::argRange.push_back(std::pair<int,int>($2,$4));
						} range_list
					;

int
					: { smecy::Attribute::isExprMode=1; smecy::Attribute::expr.str(""); } int_expr { smecy::Attribute::isExprMode=0; }
					;
					
int_expr
					: INTEGER
					| EXPR_THING { smecy::Attribute::expr << $1; } expr { std::cout << "EXPR : " << smecy::Attribute::expr.str() << std::endl; }
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
