// Assignment 4 CS 104a 
// Modified By: Konstantin Litovskiy and Gahl Levy
// Users Names: klitovsk and grlevy

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"
#include "symtable.h"

extern vector<SymbolTable*> struct_defs; 
extern vector<SymbolTable*> symbol_tables_tracker;
extern SymbolTable *global_sym_table;

struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
};

astree* new_astree (const char* lexinfo);
astree* new_astree (const char* lexinfo, astree* node);
astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree* adopt1sym (astree* root, astree* child, int symbol);
void dump_astree (FILE* outfile, astree* root);
void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep);
void free_ast (astree* tree);
void free_ast2 (astree* tree1, astree* tree2);
void astree_to_sym (astree* root);
void astree_to_oil (FILE* output, astree* root);
void type_check (SymbolTable *symTable, astree* node);
void typeCheck_astree (astree* root);
void dump_structs (FILE* output);
int lexInfoToSwitch(const char* lexinfo);

//string getTypeRec (astree* node);

string typeCheck (astree* root, int counter);
bool typeMatch (string type1, string type2);
bool isBaseType (string type);
bool isBaseTypeArr (string type);
bool isPrimative (string type);
bool isInt (string type);
bool isBool (string type);
bool isChar (string type);
bool isNULL (string type);
bool isString (string type);
bool isStruct (string name);
bool isStructIdent(string type1,string type2);
string getIdentType(string name, int counter);
string getIdentInStruct(string name);
string getTypeFromStruct(string struct_name, string name);
string getFunctionType(string name, int counter);


RCSH("$Id: astree.h,v 1.3 2013-09-20 12:23:31-07 - - $")
#endif
