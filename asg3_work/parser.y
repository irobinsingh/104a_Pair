%{
// $Id: parser.y,v 1.5 2013-10-10 18:48:18-07 - - $

// Assignment 3 CS 104a
// Authors: Konstantin Litovskiy and Gahl Levy
// Users Names: klitovsk and grlevy
    
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

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY vardcl
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE decl_list
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON expr_lsit

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token  ROOT IDENT NUMBER

%right  TOK_IF TOK_ELSE
%right  '='
%left   TOK_EQ TOK_NE TOK_LE '<' '>' TOK_GE
%left   '+' '-'
%left   '*' '/' '%'
%right    TOK_POS TOK_NEG '!' TOK_ORD TOK_CHR
%left    '[' '.'

%start  program

%%

program:    program structdef                                    { $$ = adopt1 ($1, $2); }
          | program function                                     { $$ = adopt1 ($1, $2); }
          | program statement                                    { $$ = adopt1 ($1, $2); }
          |                                                      { $$ = new_parseroot(); }
          ;

structdef:  TOK_STRUCT TOK_IDENT '{' decl_list2 '}'              { $$ = adopt2 (new_astree("structdef"), $2, $4);free_ast ($1); free_ast2($3, $5) }
          | TOK_STRUCT TOK_IDENT '{' '}'                         { $$ = adopt1 (new_astree("structdef"), $2);free_ast ($1); free_ast2($3, $4) }
          ;

decl:       type TOK_IDENT                                       { $$ = adopt1($1, $2); }
          ;
           
decl_list1:  decl                                                { $$ = $1; }
          | decl_list1 ',' decl                                  { $$ = adopt1 ($1, $3); free_ast ($2); }
          ;
          
decl_list2:  decl                                                { $$ = $1; }
          | decl_list2 ';' decl                                  { $$ = adopt1 ($1, $3); free_ast ($2); }
		  | decl_list2 ';'                                       { $$ = $1; free_ast ($2); }
          ;
       
type:       basetype TOK_ARRAY                                   { $$ = adopt1($1, $2); }
          | basetype                                             { $$ = $1; }
          ;
          
basetype:   TOK_VOID                                             { $$ = $1; }
          | TOK_BOOL                                             { $$ = $1; }
          | TOK_CHAR                                             { $$ = $1; }
          | TOK_INT                                              { $$ = $1; }
          | TOK_STRING                                           { $$ = $1; }
          | TOK_IDENT                                            { $$ = $1; }
          ;

function:   type TOK_IDENT '(' decl_list1 ')' block              { $$ = adopt2 (adopt1($1, $2), $4, $6); free_ast2($3, $5); }
          | type TOK_IDENT '(' ')' block                         { $$ = adopt1 (adopt1($1, $2), $5); free_ast2($3, $4); }
          ;
          
block:      '{' stmnt_list '}'                                   { $$ = $2; free_ast2 ($1, $3); }
          | '{' '}'                                              { free_ast2 ($1, $2);}
          | ';'                                                  { free_ast ($1);}
          ;
          
statement:  block                                                { $$ = $1; }
          | vardecl                                              { $$ = $1; }
          | while                                                { $$ = $1; }
          | ifelse                                               { $$ = $1; }
          | return                                               { $$ = $1; }
          | expr ';'                                             { $$ = $1; free_ast($2); }
          ;
          
stmnt_list: statement                                            { $$ = $1; } 
          | stmnt_list statement                                 { $$ = adopt1 ($1, $2); }
          ;
         
vardecl:    type TOK_IDENT '=' expr ';'                          { $$ = adopt1 (new_astree("vardecl"), adopt2 ($3, adopt1($1, $2), $4)); free_ast($5); }
          ;
          
while:      TOK_WHILE '(' expr ')' statement                     { $$ = adopt2 (new_astree("while"), $3, $5); free_ast2 ($1, $2); free_ast($4);}
          ;
         
ifelse:     TOK_IF '(' expr ')' statement %prec TOK_IF           { $$ = adopt2 (new_astree("if"), $3, $5); free_ast2 ($1, $2); free_ast($4); }
          | TOK_IF '(' expr ')' statement TOK_ELSE statement     { $$ = adopt2 (new_astree("if"), $3, adopt2 (new_astree("else"), $5, $7)); free_ast2 ($1, $2); free_ast2($4, $6);}
          ; 

return:     TOK_RETURN ';'                                       { $$ = new_astree ("return"); free_ast2 ($1, $2) }
          | TOK_RETURN expr ';'                                  { $$ = adopt1 (new_astree ("return"), $2); free_ast2 ($1, $3) }
          ;
          
expr:       binop                                                { $$ = $1; }
          | unop                                                 { $$ = $1; }
          | allocator                                            { $$ = $1; }
          | call                                                 { $$ = $1; }
          | '(' expr ')'                                         { $$ = $2; free_ast2 ($1, $3); }
          | variable                                             { $$ = $1; }
          | constant                                             { $$ = $1; }
          ;
          
expr_list:  expr                                                 { $$ = $1; }
          | expr_list ',' expr                                   { $$ = adopt1 ($1, $3); free_ast ($2); }
          ;

binop:      expr '=' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr TOK_EQ expr                                     { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr TOK_NE expr                                     { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr '<' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr TOK_LE expr                                     { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr '>' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr TOK_GE expr                                     { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr '+' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr '-' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr '*' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr '/' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          | expr '%' expr                                        { $$ = adopt2 (adopt1 (new_astree("binop"),$2), $1, $3); }
          ;

unop:       '+' expr %prec TOK_POS                               { $$ = adopt1sym ($1, $2, TOK_POS); }
          | '-' expr %prec TOK_NEG                               { $$ = adopt1sym ($1, $2, TOK_NEG); }
          | '!' expr                                             { $$ = adopt1sym ($1, $2, TOK_NEG); }
          | TOK_ORD expr                                         { $$ = adopt1sym ($1, $2, TOK_ORD); }
          | TOK_CHR expr                                         { $$ = adopt1sym ($1, $2, TOK_CHR); }
          ;
            
allocator:  TOK_NEW basetype '(' expr ')'                        { $$ = adopt2 ($1, $2, $4); free_ast2 ($3, $5); }
          | TOK_NEW basetype '(' ')'                             { $$ = adopt1 ($1, $2); free_ast2 ($3, $4); }
          | TOK_NEW basetype '[' expr ']'                        { $$ = adopt2 ($1, $2, $4); free_ast2 ($3, $5); }
          ;

call:       TOK_IDENT '(' expr_list ')'                          { $$ = adopt1 ($1, $3); free_ast2 ($2, $4);}
          | TOK_IDENT '(' ')'                                    { $$ = $1; free_ast2 ($2, $3);}
          ;
          
variable:   TOK_IDENT                                            { $$ = $1 }
          | expr '[' expr ']'                                    { $$ = adopt1 ($1, $3); free_ast2 ($2, $4); }
          | expr '.' TOK_IDENT                                   { $$ = adopt1 ($1, $3); free_ast ($2); }
          ;
          
constant:   TOK_INTCON                                           { $$ = $1; }
          | TOK_CHARCON                                          { $$ = $1; }
          | TOK_STRINGCON                                        { $$ = $1; }
          | TOK_FALSE                                            { $$ = $1; }
          | TOK_TRUE                                             { $$ = $1; }
          | TOK_NULL                                             { $$ = $1; }
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

