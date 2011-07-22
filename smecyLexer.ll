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
if						{ return exprMode(IF); }
stream_loop				{ return exprMode(STREAM_LOOP); }
stream_node				{ return exprMode(STREAM_NODE); }
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

/* The lexer has two "modes".
The first one is used to parse the smecy pragmas themselves
The second one is used to isolate expressions contained in the pragmas
the exprMode function will return either a smecy token (first mode) or
a EXPR_THING token (second mode)
variable smecy::Attribute::isExprMode is used to set mode
*/
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


/* This is a wrapper for the parser.
It takes the string to parse ex: "smecy map(PE,1)" and returns
a corresponding smecy::Attribute object
*/
smecy::Attribute *smecy::parseDirective(std::string directive, SgNode* parent)
{
	Attribute::currentParent = parent; //initializing static attribute (see smecyAttribute.h)
	YY_FLUSH_BUFFER; //to recover from a preceding syntax error
	
	//preparing a FILE object to be read by yyparse
	char *stream = new char[directive.size()];
	directive.copy(stream, directive.size());
	yyin = fmemopen(stream, directive.size(), "r");
	
	//actual parsing
	_yyparse(); 
	
	fclose(yyin);
	return smecy::Attribute::currentAttribute;
}
