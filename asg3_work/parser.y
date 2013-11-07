%{
// $Id: parser.y,v 1.5 2013-10-10 18:48:18-07 - - $

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "astree.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1
#define YYPRINT yyprint
#define YYMALLOC yycalloc

static void* yycalloc (size_t size);

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%destructor { error_destructor ($$); } <>

%token  ROOT IDENT NUMBER

%right  TOK_IF TOK_ELSE
%right  '='
%left   TOK_EQ TOK_NE TOK_LE '<' '>' TOK_GE
%left   '+' '-'
%left   '*' '/' '%'
%right  POS "u+" NEG "u-"    

%start  program

%%

program:    structdef*                                           { $$ = $1; }
          | function*                                            { $$ = $1; }
          | statement*                                           { $$ = $1; }
          ;       

structdef:  TOK_STRUCT IDENT '{' (decl ';')* '}'                 {          }
          ;

decl:       type IDENT                                           {          }
          ;
       
type:       basetype TOK_ARRAY                                   {          }
          ;
          
basetype:   TOK_VOID                                             {          }
          | TOK_BOOL                                             {          }
          | TOK_CHAR                                             {          }
          | TOK_INT                                              {          }
          | TOK_STRING                                           {          }
          | IDENT                                                {          }
          ;

function:   type IDENT '(' (decl( ',' decl)*) ')' block          {          }
          ;
          
block:      '{' statement* '}'                                   {          }
          | ';'                                                  {          }
          ;
          
statement:  block                                                {          }
          | vardcl                                               {          }
          | while                                                {          }
          | ifelse                                               {          }
          | return                                               {          }
          | expr ':'                                             {          }
          ;
          
vardecl:    type IDENT '=' exprt ';'                             {          }
          ;
          
while:      TOK_WHILE '(' expr ')' statement                     {          }
          ;
         
ifelse:     TOK_IF '(' expr ')' statement
          | TOK_IF '(' expr ')' statement (TOK_ELSE statement)   {          }          
          ; 

return:     TOK_RETURN ';'                                       {          }
          | TOK_RETURN expr ';'                                  {          }
          ;
          
expr:       binop                                                {          }
          | unop                                                 {          }
          | allocator                                            {          }
          | call                                                 {          }
          | unop                                                 {          }
          | '(' expr ')'                                         { free_ast2 ($1, $3); $$ = $2; }
          | variable                                             {          }
          | constant                                             {          }
          ;

binop:      expr '=' expr                                        { $$ = adopt2 ($2, $1, $3); }
		  | expr TOK_EQ expr	                                 {                           }
		  | expr TOK_NE expr	                                 {                           }
		  | expr '<' expr	                                     {                           }
		  | expr TOK_LE expr	                                 {                           }
		  | expr '>' expr	                                     {                           }
		  | expr TOK_GE expr	                                 {                           }
		  | expr '+' expr                                        { $$ = adopt2 ($2, $1, $3); }
          | expr '-' expr                                        { $$ = adopt2 ($2, $1, $3); }
          | expr '*' expr                                        { $$ = adopt2 ($2, $1, $3); }
          | expr '/' expr                                        { $$ = adopt2 ($2, $1, $3); }
          | expr '%' expr                                        { $$ = adopt2 ($2, $1, $3); }
          ;

unop:       '+' expr %prec POS                                   { $$ = adopt1sym ($1, $2, POS); }
          | '-' expr %prec NEG                                   { $$ = adopt1sym ($1, $2, NEG); }
          | '!' expr                                             { $$ = adopt1sym ($1, $2, TOK_NEG); }
          | TOK_ORD expr                                         { $$ = adopt1sym ($1, $2, TOK_ORD); }
          | TOK_CHR expr                                         { $$ = adopt1sym ($1, $2, TOK_CHR); }
          ;
            
allocator:  TOK_NEW basetype '(' 

call:       IDENT '(' expr( ',' expr)*) ')'	                    {                           }
          | IDENT '(' ')'	                                    {                           }
          ;
          
variable:   IDENT	                                            {                           }
          | expr '[' expr ']' 	                                {                           }
          | expr '.' IDENT	                                    {                           }
          ;
          
constant:  INTCON
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
        
		| '[' expr ']'          { free_ast2 ($1, $3); $$ = $2; }
        | IDENT                 { $$ = $1; }
        | NUMBER                { $$ = $1; }
        ;

%%

const char* get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != NULL);
   return result;
}

RCSC("$Id: parser.y,v 1.5 2013-10-10 18:48:18-07 - - $")

