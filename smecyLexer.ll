/* SMECY pragma lexer */

%{
#include <stdlib.h>
#include <string>
#include <iostream>
#include "smecyParser.tab.hh"
#include "smecyAttribute.h"
void yyerror(char *);
int _yyparse();
%}

%%
smecy					{ return SMECY; }
map						{ return MAP; }
arg						{ return ARG; }
in						{ return IN; }
out						{ return OUT; }
inout					{ return INOUT; }
unused					{ return UNUSED; }
[:,\[\]()/]				{ return *yytext; }
[1-9][0-9]*				{ yylval.intType = atoi(yytext);
						  return INTEGER; }
[a-zA-Z_][-a-zA-Z0-9_]*	{ std::cout << "found id" << std::endl;
						  yylval.stringType = strdup(yytext);
						  return ID; }
[ \t\n]+				{ ; }
.						{ yyerror((char *)"Unknown character"); }
%%

int yywrap(void)
{
	return 1;
}

int main(int argc, char *argv[])
{
	yyin = fopen(argv[1],"r");
	_yyparse();
	fclose(yyin);
	
	yyin = fopen(argv[1],"r");
	_yyparse();
	fclose(yyin);
}
