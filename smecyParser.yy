/* SMECY pragma parser */

%{
#include <iostream>

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

%start smecy_directive;

%%
smecy_directive
					: SMECY arg_clause arg_clause_list	//we do not allow empty directives
					| SMECY map_clause arg_clause_list
					;

arg_clause
					: ARG '(' INTEGER ',' arg_parameter_list ')'
					;
					
arg_clause_list		
					: // empty 
					| arg_clause arg_clause_list
					;
					
map_clause
					: MAP '(' ID closing_map_clause
					;
					
closing_map_clause
					: ')'
					| ',' INTEGER ')'
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
					: IN
					| OUT
					| INOUT
					| UNUSED
					| size
					| range
					;
					
size
					: '[' INTEGER ']' size_list	//FIXME allow expressions?
					;

size_list
					: //empty
					| '[' INTEGER ']' size_list
					;

range
					: '/' '[' INTEGER ':' INTEGER ']' range_list
					;
					
range_list
					: //empty
					| '[' INTEGER ':' INTEGER ']' range_list
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
