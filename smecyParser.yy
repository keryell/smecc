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
%token <stringType> ID;

%type <intType> closing_map_clause

%start smecy_directive;

%%
smecy_directive
					: SMECY { smecyAttribute::attributeBeingBuilt=new smecyAttribute(); } arg_clause arg_clause_list	//we do not allow empty directives
					| SMECY { smecyAttribute::attributeBeingBuilt=new smecyAttribute(); } map_clause arg_clause_list
					;

//arg_clause puts the arg number on top of the stack to be passed down
arg_clause
					: ARG '(' INTEGER ',' { smecyAttribute::argNumber = $3; } arg_parameter_list ')'
					;
					
arg_clause_list
					: // empty 
					| arg_clause arg_clause_list
					;
					
map_clause
					: MAP '(' ID closing_map_clause { 
						smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause($3,$4));
						}
					;
					
closing_map_clause
					: ')' { $$ = -1; }
					| ',' INTEGER ')' { $$ = $2; }
					;
					
/*outer_expression	//we want to avoid confusion between the parenthesis of the expression and of the clause
					: NOPAR
					| NOPAR '(' inner_expression ')'NOPAR	//FIXME shift/reduce conflict
					;
					
inner_expression
					: NOPAR
					| inner_expression ',' inner_expression
					| NOPAR '(' inner_expression ')'NOPAR
					;*/
					
arg_parameter_list
					: arg_parameter
					| arg_parameter ',' arg_parameter_list
					;
					
arg_parameter		
					: IN { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause(smecyAttribute::argNumber,smecy_arg_in)); }
					| OUT { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause(smecyAttribute::argNumber,smecy_arg_out)); }
					| INOUT { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause(smecyAttribute::argNumber,smecy_arg_inout)); }
					| UNUSED { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause(smecyAttribute::argNumber,smecy_arg_unused)); }
					| size
					| range
					;
					
size
					: '[' INTEGER ']' { smecyAttribute::argSize.clear();
						smecyAttribute::argSize.push_back($2);
						} size_list
					; //FIXME allow expressions?

size_list
					: /* empty */ { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause(smecyAttribute::argNumber,smecyAttribute::argSize)); }
					| '[' INTEGER ']' { smecyAttribute::argSize.push_back($2); } size_list
					;

range
					: '/' '[' INTEGER ':' INTEGER ']' { smecyAttribute::argRange.clear();
						smecyAttribute::argRange.push_back(std::pair<int,int>($3,$5));
						} range_list	//TODO add possibility for empty or "single" ranges
					;
					
range_list
					: /* empty */ { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause(smecyAttribute::argNumber,smecyAttribute::argRange)); }
					| '[' INTEGER ':' INTEGER ']' { smecyAttribute::argRange.push_back(std::pair<int,int>($2,$4));
						} range_list
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
