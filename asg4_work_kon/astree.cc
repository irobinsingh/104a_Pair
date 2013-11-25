// Assignment 3 CS 104a 
// Modified By: Konstantin Litovskiy and Gahl Levy
// Users Names: klitovsk and grlevy

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"


astree* new_astree (const char* lexinfo) {
	return new_astree (0,0,0,0,lexinfo);
}

astree* new_astree (const char* lexinfo, astree* node) {
	return new_astree (0,node->filenr,node->linenr,node->offset,lexinfo);
}

astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = intern_stringset (lexinfo);
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo->c_str());
   return tree;
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

static void dump_node (FILE* outfile, astree* node) {
   if(node->symbol == 0){
		fprintf (outfile, "%s", node->lexinfo->c_str());
   }else{
		fprintf (outfile, "%s (%s)", get_yytname (node->symbol), node->lexinfo->c_str());
   }   
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   fprintf (outfile, "%*s", depth * 4, "");
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child) {
      dump_astree_rec (outfile, root->children[child], depth + 1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep) {
   DEBUGF ('f', "toknum = %d, yyvaluep = %p\n", toknum, yyvaluep);
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n", get_yytname (toknum), toknum);
   }
   fflush (NULL);
}

void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
    // prints debug info
   DEBUGF ('f', "free [%X]-> %d:%d.%d: %s: \"%s\")\n",
           (uintptr_t) root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

static void node_to_sym (SymbolTable *symTable, astree* node) {
	string vardecl = "vardecl";
	string function = "function";
	string statement = "statement";
	string block = "block";
	string while_s = "while";
	string if_s = "if";
	string else_s = "if";
   
    if(strcmp(node->lexinfo->c_str(), block.c_str()) == 0 || strcmp(node->lexinfo->c_str(), while_s.c_str()) == 0 || strcmp(node->lexinfo->c_str(), if_s.c_str()) == 0  || strcmp(node->lexinfo->c_str(), else_s.c_str()) == 0 || strcmp(node->lexinfo->c_str(), statement.c_str()) == 0){
		if(strcmp(node->lexinfo->c_str(), statement.c_str()) == 0){
			printf("****statement\n");
			symTable = symTable->enterBlock();
		}
		if(strcmp(node->lexinfo->c_str(), block.c_str()) == 0){
			printf("****block\n");
			symTable = symTable->enterBlock();
		}
		if(strcmp(node->lexinfo->c_str(), while_s.c_str()) == 0){
			printf("****while\n");
			symTable = symTable->enterBlock();
		}
		if(strcmp(node->lexinfo->c_str(), if_s.c_str()) == 0){
			printf("****if\n");
			symTable = symTable->enterBlock();
		}
		
		if(strcmp(node->lexinfo->c_str(), else_s.c_str()) == 0){
			printf("****else\n");
			symTable = symTable->enterBlock();
		}
		
	}else{
		if (strcmp(node->lexinfo->c_str(), vardecl.c_str()) == 0){
			if(node->children[0]->children[0]->children[0]->symbol == TOK_ARRAY){
				printf("%s _ %d,%d,%d _ %s%s\n", 
					node->children[0]->children[0]->children[1]->lexinfo->c_str(), 
					node->filenr, 
					node->linenr, 
					node->offset, 
					node->children[0]->children[0]->lexinfo->c_str(),
					node->children[0]->children[0]->children[0]->lexinfo->c_str());
				
				string tempStr = node->children[0]->children[0]->lexinfo->c_str();
				tempStr.append(node->children[0]->children[0]->children[0]->lexinfo->c_str());
				
				symTable->addSymbol(node->children[0]->children[0]->children[1]->lexinfo->c_str(), tempStr);
			}else{
				printf("%s _ %d,%d,%d _ %s\n",
					node->children[0]->children[0]->children[0]->lexinfo->c_str(),
					node->filenr,
					node->linenr,
					node->offset,
					node->children[0]->children[0]->lexinfo->c_str());
				
				symTable->addSymbol(node->children[0]->children[0]->children[0]->lexinfo->c_str(),
						node->children[0]->children[0]->lexinfo->c_str());
			}
			symTable->addSymbol("test","test");
			//symTable->addSymbol(node->children[0]->children[0]->children[0]->lexinfo->c_str(), strcat(strcat("",node->children[0]->children[0]->lexinfo->c_str()), node->children[0]->children[0]->children[0]->lexinfo->c_str()));
		}else{
			if (strcmp(node->lexinfo->c_str(), function.c_str()) == 0){
				if(node->children[0]->children[0]->children[0]->symbol == TOK_ARRAY){
				printf("%s _ %d,%d,%d _ %s%s()\n\t", 
					node->children[0]->children[0]->children[1]->lexinfo->c_str(), 
					node->filenr, 
					node->linenr, 
					node->offset, 
					node->children[0]->children[0]->lexinfo->c_str(),
					node->children[0]->children[0]->children[0]->lexinfo->c_str());
				}else{
					printf("%s _ %d,%d,%d _ %s()\n\t",
						node->children[0]->children[0]->children[0]->lexinfo->c_str(),
						node->filenr,
						node->linenr,
						node->offset,
						node->children[0]->children[0]->lexinfo->c_str());
				}
				//symTable.addSymbol(node->children[0]->children[0]->children[0]->lexinfo, node->filenr, node->linenr, node->offset, node->children[0]->children[0]->lexinfo);
			}
		}
	}
}

static void astree_to_sym_rec (SymbolTable *symTable, astree* root, int depth) {
   if (root == NULL) return;
   //fprintf (outfile, "%*s", depth * 4, "");
   node_to_sym (symTable, root);
   //fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child) {
      astree_to_sym_rec (symTable, root->children[child], depth + 1);
   }
}

void astree_to_sym (SymbolTable *symTable, astree* root) {
   astree_to_sym_rec(symTable, root, 0);
   fflush (NULL);
}

RCSC("$Id: astree.cc,v 1.14 2013-10-10 18:48:18-07 - - $")

