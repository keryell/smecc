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

%type <intType> closing_map_clause arg_clause

%start smecy_directive;

%%
smecy_directive
					: SMECY { smecyAttribute::attributeBeingBuilt=new smecyAttribute(); } arg_clause arg_clause_list	//we do not allow empty directives
					| SMECY { smecyAttribute::attributeBeingBuilt=new smecyAttribute(); } map_clause arg_clause_list
					;

//arg_clause puts the arg number on top of the stack to be passed down
arg_clause
					: ARG '(' INTEGER ',' arg_parameter_list { $<intType>$ = $3; } ')'
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
					: arg_parameter { $<intType>$ = $<intType>0; }
					| arg_parameter { $<intType>$ = $<intType>0; } ',' arg_parameter_list
					;
					
arg_parameter		
					: IN { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause($<intType>0,smecy_arg_in)); }
					| OUT { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause($<intType>0,smecy_arg_out)); }
					| INOUT { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause($<intType>0,smecy_arg_inout)); }
					| UNUSED { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause($<intType>0,smecy_arg_unused)); }
					| { $<intType>$ = $<intType>0; } size
					| { $<intType>$ = $<intType>0; } range
					;
					
size
					: '[' INTEGER ']' { $<intType>$ = $<intType>0;
						smecyAttribute::argSize.clear();
						smecyAttribute::argSize.push_back($2);
						} size_list
					; //FIXME allow expressions?

size_list
					: /* empty */ { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause($<intType>0,smecyAttribute::argSize)); }
					| '[' INTEGER ']' { $<intType>$ = $<intType>0;
						smecyAttribute::argSize.push_back($2);
						} size_list
					;

range
					: '/' '[' INTEGER ':' INTEGER ']' { $<intType>$ = $<intType>0;
						smecyAttribute::argRange.clear();
						smecyAttribute::argRange.push_back(std::pair<int,int>($3,$5));
						} range_list	//TODO add possibility for empty or "single" ranges
					;
					
range_list
					: /* empty */ { smecyAttribute::attributeBeingBuilt->addSmecyClause(smecyClause($<intType>0,smecyAttribute::argRange)); }
					| '[' INTEGER ':' INTEGER ']' { $<intType>$ = $<intType>0;
						smecyAttribute::argRange.push_back(std::pair<int,int>($2,$4));
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
