/* SMECY pragma parser */

%{
#include "public.hpp"
#include "smecyAttribute.hpp"

using namespace smecy;

int yylex(void);
void yyerror(char *);
int _yyparse();
%}

%union
{
  int intType;
  const char *stringType;
}

%token SMECY MAP ARG '[' ']' '/' '(' ')' ':' ',' IN OUT INOUT UNUSED
%token COMMUNICATION SRC DST STREAM_LOOP STAGE IF LABEL
%token <intType> INTEGER
%token <stringType> ID EXPRESSION
%type <stringType> proc_id

%start smecy_directive
%debug
%verbose

%%

/* a SME-C directive is a list of clauses

   The static attribute object that will be used to store the result of
   the parsing is initialized

   A list meant to contain all C expressions contained in the directive is
   also initialized

   All this parser should be reworked to be in the classic style, using $$
   to build data structures on the flight and be reentrant instead of
   using a lot of static data structures
*/

smecy_directive          : SMECY
                           { Attribute::currentExpressionList.clear();
                             Attribute::currentAttribute = new Attribute(Attribute::currentParent); }
                           clause_list
                           { Attribute::currentAttribute->setExpressionList(Attribute::currentExpressionList); }
                         ;


clause_list              : /*empty*/
                         | map_clause clause_list
                         | arg_clause clause_list
                         | if_clause clause_list
                         | stream_loop_clause clause_list
                         | stage_clause clause_list
                         | label_clause clause_list
                         | communication_clause clause_list
                         ;


/* From there, we use the various methods of Attribute objects (see
   smecyAttribute.hpp) to add the information regarding the different
   types of clause */
stream_loop_clause       : STREAM_LOOP
                           { Attribute::currentAttribute->addStreamLoop(); }
                         ;


stage_clause             : STAGE { Attribute::currentAttribute->addStage(); }
                         ;


label_clause             : LABEL '(' INTEGER
                           { smecyLexerPushExpressions(); }
                           ')'
                           { Attribute::currentAttribute->addLabel($3); }
                         ;


/* TODO: use  $$ = ... instead of using global variables such as
   arg_ranges.push_back(), Attribute::currentIntExpr...
*/
if_clause                : IF '('
                           { smecyLexerPushExpressions(); }
                           expression ')'
                           { Attribute::currentAttribute->addIf(Attribute::currentIntExpr); }
                         /* some computation is hidden in int*/
                         ;


arg_clause
                         : ARG '(' INTEGER ','
                           { Attribute::argNumber = $3; }
                           arg_parameter_list
                           ')'
                           /* some computation is hidden in arg_parameter_list*/
                         ;


map_clause               : MAP '(' proc_id ')'
                           { Attribute::currentAttribute->addMap($3, Attribute::args);
                           }
                         ;


proc_id                  : ID
                           { /* Do not accept "," as part of the
                                  expression here up to next scoping ")" */
                             smecyLexerPushExpressionsWithoutComma();
                           }
                           optional_expression_list
                           { $$ = $1; }
                         ;


/* Do nothing yet, but at least no syntax error on communication pragma */
communication_clause     : COMMUNICATION
                           communication_src_and_dst
                         ;

/* Accept src and dst in both order */
communication_src_and_dst: SRC memory_desc DST memory_desc
                         | DST memory_desc SRC memory_desc
                         ;


memory_desc              : '(' ID ',' proc_id ')'
                         ;


arg_parameter_list       : arg_parameter
                         | arg_parameter ',' arg_parameter_list
                         /* some computation is hidden in arg_parameter*/
                         ;


arg_parameter            : IN { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_in); }
                         | OUT { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_out); }
                         | INOUT { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_inout); }
                         | UNUSED { Attribute::currentAttribute->addArg(Attribute::argNumber,_arg_unused); }
                         | size
                         | range
                         /* some computation is hidden in size and range*/
                         ;

/* TODO: simplify size & size_list */
size                     : '['
                           { smecyLexerPushExpressionsWithoutColon(); }
                           expression ']'
                           { Attribute::args.clear();
                             Attribute::args.push_back(Attribute::currentIntExpr);
                           }
                           size_list
                         /* some computation is hidden in size_list */
                         ;


size_list                : /*empty*/
                           { Attribute::currentAttribute->addArg(Attribute::argNumber, Attribute::args);
                           }
                         | '['
                           { smecyLexerPushExpressionsWithoutColon(); }
                           expression ']'
                           { Attribute::args.push_back(Attribute::currentIntExpr);
                           }
                           size_list
                         ;


optional_expression_list : /*empty*/
                           { Attribute::args.clear(); }
                           | ',' expression_list;


// Parse into args a list of integer expressions separated by ',':
expression_list          : expression {
                             // Clear the current list of
                             // arguments
                             Attribute::args.clear();
                             // and initialize with the
                             // first int expression
                             Attribute::args.push_back(Attribute::currentIntExpr);
                           }
                           more_expression_list
                         ;


more_expression_list     : /*empty*/
                         | ',' expression
                           { Attribute::args.push_back(Attribute::currentIntExpr); }
                           more_expression_list
                         ;

/* range is divided in several parts to cover the different cases ([], [n] and [n:m])
a range is made of pairs of intExpr with the following correspondance :
[] -> (-1,-1)
[n] -> (n,-1)
[n:m] -> (n,m)
  FIXME Simplify the following
*/
range                    : '/' '['
                           { smecyLexerPushExpressionsWithoutColon();
                             Attribute::argRange.clear();
                           }
                           range_begin
                         ;


range_begin              : expression
                           { Attribute::currentPair.first = Attribute::currentIntExpr; }
                           range_mid
                         | ']'
                           {
                             Attribute::argRange.push_back(std::pair<IntExpr,IntExpr>(IntExpr(-1),IntExpr(-1)));
                           }
                           range_open
                         ;


range_mid                : ':' expression
                           { Attribute::currentPair.second = Attribute::currentIntExpr;
                             Attribute::argRange.push_back(Attribute::currentPair);
                           }
                           ']' range_open
                         | ']'
                           { Attribute::currentPair.second = IntExpr(-1);
                             Attribute::argRange.push_back(Attribute::currentPair);
                           }
                           range_open
                         ;


range_open               : /*empty*/
                           { Attribute::currentAttribute->addArg(Attribute::argNumber,Attribute::argRange);}
                         | '['
                           { smecyLexerPushExpressionsWithoutColon(); }
                           range_begin
                         ;


/* Use the EXPRESSIONS, WITHOUT_COLON or WITHOUT_COMMA start mode in the
   lexer to be able to parse expression in an "opaque" mode without having
   to understand all the C syntax.

   In this mode the lexer will return EXPRESSION for everything except
   ()[] If in mode WITHOUT_COLON or WITHOUT_COMMA ,: are delimiters
   respectively

   Parenthesis and brackets are counted so as to make sure the expression
   does not include parts of the directive
*/
expression               : { Attribute::expr.str(""); }
                           expr
                           { /* Set the current internal expression to
                                what we parsed in this expression: */
                             Attribute::currentIntExpr = IntExpr(Attribute::expr.str());
                           }
                         ;


/* To verify the matching ( ) */
expr                     : expr expr
                         | EXPRESSION
                           { Attribute::expr << $1; }
                         | '('
                           { Attribute::expr << '('; }
                           expr ')'
                           { Attribute::expr << ')'; }
                         ;

%%

void yyerror(char *s) {
  std::cerr << s << std::endl;
}


/* function yyparse needs to be wrapped so as to be used in another file
   (see smecyLexer.ll)
*/
int _yyparse() {
  //yydebug = 1;
  return yyparse();
}
