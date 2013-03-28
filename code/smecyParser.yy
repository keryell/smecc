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
   the parsing is initialized.

   A list meant to contain all C expressions contained in the directive is
   also initialized.

   All this parser should be reworked to be in the classic style, using $$
   to build data structures on the flight and be reentrant instead of
   using a lot of static data structures


   The design ideas:

   - the pragma is parsed from attachAttributes() into a new
     Attribute::currentAttribute which is allocated here

   - Attribute::currentAttribute are filled with the various components

   - a string version of each expression is kept into
     Attribute::expressionList

   - this Attribute::expressionList is later parsed from
     parseExpressions() by ROSE in the program context to
     Attribute::sgExpressionList so that each expression is an AST object
     and can be used in the code generation process
*/

smecy_directive          : SMECY
                           { /* Reset the list of the expressions
                                associated to this pragme */
                             Attribute::currentExpressionList.clear();
                             /* This pragma is associated to the statement
                                we deal with */
                             Attribute::currentAttribute = new Attribute(Attribute::currentParent);
                           }
                           clause_list
                           { /* Keep the list of each individual top
                                expression to parse it later in the
                                context of the whole program */
                             Attribute::currentAttribute->setExpressionList(Attribute::currentExpressionList); }
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
                           { Attribute::args.push_back(Attribute::currentIntExpr); }
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
                             // Initialize with the first expression
                             Attribute::args.push_back(Attribute::currentIntExpr);
                           }
                           more_expression_list
                         ;


more_expression_list     : /*empty*/
                         | ',' expression
                           { Attribute::args.push_back(Attribute::currentIntExpr); }
                           more_expression_list
                         ;

/* A range is divided in several parts to cover the different cases ([],
   [n] and [n:m]).

   A range is made of pairs of intExpr with the following matching :
   - [] -> (-1,-1)
   - [n] -> (n,-1)
   - [n:m] -> (n,m)

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
                             /* Empty range found */
                             Attribute::argRange.push_back(std::pair<IntExpr,IntExpr> {-1, -1});
                           }
                           range_open
                         ;


range_mid                : ':' expression
                           { Attribute::currentPair.second = Attribute::currentIntExpr;
                             Attribute::argRange.push_back(Attribute::currentPair);
                           }
                           ']' range_open
                         | ']'
                           { /* No ":m" */
                             Attribute::currentPair.second = { -1 };
                             Attribute::argRange.push_back(Attribute::currentPair);
                           }
                           range_open
                         ;


range_open               : /*empty*/
                           { /* Keep track of the current range that has
                                been parsed */
                             Attribute::currentAttribute->addArg(Attribute::argNumber,Attribute::argRange); }
                         | '['
                           { smecyLexerPushExpressionsWithoutColon();
                             /* Parse the next [...] */
                           }
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
                                what we parsed in this expression so that
                                it can be used later by other rules: */
                             Attribute::currentIntExpr = IntExpr(Attribute::expr.str());
                             /* Add the current string expression to the
                                list of expressions to be parsed by ROSE
                                in the program context later */
                             Attribute::currentExpressionList.push_back(Attribute::expr.str());
                           }
                         ;


/* To verify the matching ( ) and gather the expression into
   Attribute::expr */
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
