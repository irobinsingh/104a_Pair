%{
// $Id: parser.y,v 1.5 2013-10-10 18:48:18-07 - - $

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "astree.h"

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

decl:       type IDENT                                           { $$ = adopt1($1, $2); }
          ;
       
type:       basetype TOK_ARRAY                                   { $$ = adopt1($1, $2); }
          ;
          
basetype:   TOK_VOID                                             { $$ = $1; }
          | TOK_BOOL                                             { $$ = $1; }
          | TOK_CHAR                                             { $$ = $1; }
          | TOK_INT                                              { $$ = $1; }
          | TOK_STRING                                           { $$ = $1; }
          | IDENT                                                {          }
          ;

function:   type IDENT '(' (decl( ',' decl)*) ')' block          {          }
          ;
          
block:      '{' statement* '}'                                   {          }
          | ';'                                                  {          }
          ;
          
statement:  block                                                { $$ = $1; }
          | vardcl                                               { $$ = $1; }
          | while                                                { $$ = $1; }
          | ifelse                                               { $$ = $1; }
          | return                                               { $$ = $1; }
          | expr ':'                                             { $$ = $1; free_ast($2); }
          ;
          
vardecl:    type IDENT '=' expr ';'                             { $$ =            }
          ;
          
while:      TOK_WHILE '(' expr ')' statement                     { $$ = adopt2 ($1, $3, $5); free_ast2 ($2, $4);}
          ;
         
ifelse:     TOK_IF '(' expr ')' statement                        { $$ = adopt2 ($1, $3, $5); free_ast2 ($2, $4); }
          | TOK_IF '(' expr ')' statement (TOK_ELSE statement)   { $$ = adopt2 (adopt1sym ($1, $3, TOK_IFELSE), $5, $7);freeast3 ($2, $4, $6); }
          ; 

return:     TOK_RETURN ';'                                       { $$ = adopt1 ($1, NULL); free_ast ($2) }
          | TOK_RETURN expr ';'                                  { $$ = adopt1 ($1, $2); free_ast ($3) }
          ;
          
expr:       binop                                                { $$ = $1; }
          | unop                                                 { $$ = $1; }
          | allocator                                            { $$ = $1; }
          | call                                                 { $$ = $1; }
          | unop                                                 { $$ = $1; }
          | '(' expr ')'                                         { free_ast2 ($1, $3); $$ = $2; }
          | variable                                             { $$ = $1; }
          | constant                                             { $$ = $1; }
          ;

binop:      expr '=' expr                                        { $$ = adopt2 ($2, $1, $3); }
		  | expr TOK_EQ expr	                                 { $$ = adopt2 ($2, $1, $3); }
		  | expr TOK_NE expr	                                 { $$ = adopt2 ($2, $1, $3); }
		  | expr '<' expr	                                     { $$ = adopt2 ($2, $1, $3); }
		  | expr TOK_LE expr	                                 { $$ = adopt2 ($2, $1, $3); }
		  | expr '>' expr	                                     { $$ = adopt2 ($2, $1, $3); }
		  | expr TOK_GE expr	                                 { $$ = adopt2 ($2, $1, $3); }
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
            
allocator:  TOK_NEW basetype '(' expr ')'
          | TOK_NEW basetype '(' ')'
          | TOK_NEW basetype '[' expr ']'
          ;

call:       IDENT '(' expr( ',' expr)*) ')'	                    {                           }
          | IDENT '(' ')'	                                    {                           }
          ;
          
variable:   IDENT	                                            {                           }
          | expr '[' expr ']' 	                                {                           }
          | expr '.' IDENT	                                    {                           }
          ;
          
constant:   TOK_INTCON
          | TOK_CHARCON
          | TOK_STRINGCON
          | TOK_FALSE
          | TOK_TRUE
          | TOK_NULL
          ;
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
        
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

