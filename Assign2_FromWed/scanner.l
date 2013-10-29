%{
// $Id: scanner.l,v 1.3 2013-10-15 16:37:56-07 - - $

#include "auxlib.h"
#include "lyutils.h"

#define YY_USER_ACTION  { scanner_useraction (); }
#define IGNORE(THING)   { }

%}

%option 8bit
%option debug
%option ecs
%option nodefault
%option nounput
%option noyywrap
%option perf-report
%option verbose
%option warn

LETTER          [A-Za-z_]
DIGIT           [0-9]
MANTISSA        ({DIGIT}+\.?{DIGIT}*|\.{DIGIT}+)
EXPONENT        ([Ee][+-]?{DIGIT}+)
NUMBER          ({MANTISSA}{EXPONENT}?)
NOTNUMBER       ({MANTISSA}[Ee][+-]?)
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
NOTIDENT		({NUMBER}{IDENT})
CHAR			(’([^\\’\n]|\\[\\’"0nt])’)
STRING			(\"([^\\"\n]|\\[\\’"0nt])*\")

%%

"#".*           { scanner_include(); }
[ \t]+          { IGNORE (white space) }
\n              { scanner_newline(); }

void			{return yylval_token (TOK_VOID); }
bool			{return yylval_token (TOK_BOOL); }
char			{return yylval_token (TOK_CHAR); }
int				{return yylval_token (TOK_INT); }
string			{return yylval_token (TOK_STRING); }
struct			{return yylval_token (TOK_STRUCT); }
if				{return yylval_token (TOK_IF); }
else			{return yylval_token (TOK_ELSE); }
while			{return yylval_token (TOK_WHILE); }
return			{return yylval_token (TOK_RETURN); }
new				{return yylval_token (TOK_NEW); }
true			{return yylval_token (TOK_TRUE); }
false			{return yylval_token (TOK_FALSE); }
null			{return yylval_token (TOK_NULL); }
ord				{return yylval_token (TOK_ORD); }
chr				{return yylval_token (TOK_CHR); }

{NUMBER}        { return yylval_token (TOK_INTCON); }
{IDENT}         { return yylval_token (TOK_IDENT); }
{CHAR}			{ return yylval_token (TOK_CHARCON); }
{STRING}		{ return yylval_token (TOK_STRINGCON); }

"="             { return yylval_token ('='); }
"=="			{ return yylval_token (TOK_EQ); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"^"             { return yylval_token ('^'); }
"[]"			{ return yylval_token (TOK_ARRAY);}
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']'); }
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
";"             { return yylval_token (';'); }
"<"				{ return yylval_token ('<'); }
"<="			{ return yylval_token (TOK_LE); }
">"				{ return yylval_token ('>'); }
">="			{ return yylval_token (TOK_GE); }

{NOTNUMBER}     { scanner_badtoken (yytext);
                  return yylval_token (TOK_INTCON); }

.               { scanner_badchar (*yytext); }

%%

RCSC("$Id: scanner.l,v 1.3 2013-10-15 16:37:56-07 - - $")

