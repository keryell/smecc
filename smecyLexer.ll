/* SMECY pragmas lexer */

%{
#include <stdlib.h>
#include <string>
#include <iostream>
#include "smecyParser.tab.hh"
void yyerror(char *);
int _yyparse();
%}

%%
smecy			{
					std::cout << "found smecy" << std::endl;
					return SMECY;
				}
map				{
					std::cout << "found map" << std::endl;
					return MAP;
				}
arg				return ARG;
in				{
					std::cout << "found in" << std::endl;
					return IN;
				}
out				return OUT;
inout			return INOUT;
unused			return UNUSED;
[:,()]		return *yytext;		//FIXME check for the []
[1-9][0-9]*		{
					yylval.intType = atoi(yytext);
					return INTEGER;
				}
[a-zA-Z_][-a-zA-Z0-9_]*	{
					std::cout << "found id" << std::endl;
					yylval.stringType = strdup(yytext);
					return ID;
				}
[ \t\n]+		;
.				yyerror((char *)"Unknown character");
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
} 
