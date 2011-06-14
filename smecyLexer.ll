/* SMECY pragma lexer */

%{
#include "public.h"
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

int parseSmecyDirective(std::string directive)
{
	//yyin = fopen(directive.data(),"r");
	char *stream = new char[directive.size()];
	directive.copy(stream, directive.size());
	yyin = fmemopen(stream, directive.size(), "r");
	_yyparse();
	fclose(yyin);
	return 0;
}
