/* SMECY pragma lexer */

%{
#include "public.h"
#include "smecyParser.tab.hh"
#include "smecyAttribute.h"

void yyerror(char *);
int _yyparse();
int exprMode(int);
%}

%option nounput

%%
smecy					{ return exprMode(SMECY); }
map						{ return exprMode(MAP); }
arg						{ return exprMode(ARG); }
in						{ return exprMode(IN); }
out						{ return exprMode(OUT); }
inout					{ return exprMode(INOUT); }
unused					{ return exprMode(UNUSED); }
[\[\](),:]				{ return *yytext; }
[/]						{ return exprMode(*yytext); }
[0-9]+					{ yylval.intType = atoi(yytext);
						  return INTEGER; }
[a-zA-Z_][-a-zA-Z0-9_]*	{ yylval.stringType = strdup(yytext);
						  return exprMode(ID); }
[ \\\t\n]+				{ ; }
.						{ if (!smecy::Attribute::isExprMode)
							yyerror((char *)"Unknown character");
						  else
						  {
						  	yylval.stringType = strdup(yytext);
							return EXPR_THING;
						  }
						}
%%

int yywrap(void)
{
	return 1;
}

int exprMode(int token)
{
	if (smecy::Attribute::isExprMode)
	{
		yylval.stringType = strdup(yytext);
		return EXPR_THING;
	}
	else
		return token;
}

smecy::Attribute *smecy::parseDirective(std::string directive)
{
	char *stream = new char[directive.size()];
	directive.copy(stream, directive.size());
	YY_FLUSH_BUFFER;
	yyin = fmemopen(stream, directive.size(), "r");
	_yyparse();
	fclose(yyin);
	return smecy::Attribute::currentAttribute;
}
