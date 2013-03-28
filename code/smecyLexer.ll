/* SMECY pragma lexer */

%{
#include "public.hpp"
#include "smecyParser.tab.hh"
#include "smecyAttribute.hpp"

void yyerror(char *);
int _yyparse();
%}

%option nounput
%option debug
%option stack

 /* Define another start mode used to scan for opaque expressions inside
    #pragma smecy. It is an inclusive mode because separators are to be
    detected in both mode.
 */
%s EXPRESSIONS
%s WITHOUT_COMMA
%s WITHOUT_COLON

%%

  /* The #pragma keywords
   */
<INITIAL>{
  smecy                   { return SMECY; }
  map                     { return MAP; }
  arg                     { return ARG; }
  if                      { return IF; }
  stream_loop             { return STREAM_LOOP; }
  stage                   { return STAGE; }
  label                   { return LABEL; }
  in                      { return IN; }
  out                     { return OUT; }
  inout                   { return INOUT; }
  unused                  { return UNUSED; }
  communication           { return COMMUNICATION; }
  src                     { return SRC; }
  dst                     { return DST; }
  /* Some separator with a special meaning at the top level. The ")" is
     here too to avoid some yy_pop_state() underflow in some cases when we
     are still in INITIAL state: */
  [][/,:)(]                 { return *yytext; }
  /* An identifier for a processor or memory: */
  [a-zA-Z_][-a-zA-Z0-9_]* { yylval.stringType = strdup(yytext);
                            return ID; }
  /* An integer: */
  [0-9]+                  { yylval.intType = atoi(yytext);
                            return INTEGER;
                          }
}

[])]                      { /* Restore the previous start state */
                            yy_pop_state();
                            fprintf(stderr, "Back to state %d\n", YY_START);
                            return *yytext;
                          }
<EXPRESSIONS,WITHOUT_COLON,WITHOUT_COMMA>{
  "("                     { /* After a parenthesis, we allow "," in
                               expressions */
                            yy_push_state(EXPRESSIONS);
                            return *yytext;
                          }
}

<WITHOUT_COLON>:          { /* Do not return it as a part of an EXPRESSION */
                             return *yytext; }

<WITHOUT_COLON>,          { /* To go back to initial state when parsing
                               arg() */
                             return *yytext; }

<WITHOUT_COMMA>,          { /* Do not return it as a part of an EXPRESSION */
                             return *yytext; }

[ \\\t\n]+                /* Blanks are not significant and are only
                             separators */

  /* In expression mode, slurp up to the next separator to form an opaque
     expression content we will provide back to ROSE for further */
.                         { yylval.stringType = strdup(yytext);
                            return EXPRESSION;
                          }
%%


int yywrap(void) {
  /* Do not read another file when we reach the end. Since we scan only a
     string at a time, it does not make sense... */
  return 1;
}


/* To set the start mode from the bison parser*/
void smecyLexerStartINITAL() {
  fprintf(stderr, "Going to INITIAL (%d) state\n", YY_START);
  BEGIN(INITIAL);
}

void smecyLexerPushExpressions() {
  fprintf(stderr, "Going to EXPRESSIONS (%d) state\n", YY_START);
  yy_push_state(EXPRESSIONS);
}


void smecyLexerPushExpressionsWithoutColon() {
  yy_push_state(WITHOUT_COLON);
  fprintf(stderr, "Going to WITHOUT_COLON (%d) state\n", YY_START);
}


void smecyLexerPushExpressionsWithoutComma() {
  yy_push_state(WITHOUT_COMMA);
  fprintf(stderr, "Going to WITHOUT_COMMA (%d) state\n", YY_START);
}


/* This is the wrapper for the parser.

   It takes the string to parse ex: "smecy map(PE,1)" and returns
   a corresponding smecy::Attribute object
*/
smecy::Attribute *smecy::parseDirective(std::string directive, SgNode* parent) {
  // Initializing static attribute (see smecyAttribute.h)
  Attribute::currentParent = parent;
  /* Flushes the scanner's internal buffer to recover from a preceding
     syntax error */
  YY_FLUSH_BUFFER;
  smecyLexerStartINITAL();

  // Preparing a FILE object to be read by yyparse()
  char *stream = new char[directive.size()];
  directive.copy(stream, directive.size());
  yyin = fmemopen(stream, directive.size(), "r");
  yy_flex_debug = 1;
  // Actual parsing
  int error_code = _yyparse();
  fclose(yyin);

  if (error_code != 0)
    /* Uplift the current text as an exception
       if there is an error during parsing */
    throw yytext;

  return smecy::Attribute::currentAttribute;
}
