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
   //fprintf (outfile, "%s (%s)", get_yytname (node->symbol), node->lexinfo->c_str());
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
   dump_astree_rec (stdout, root, 0);
   fflush (NULL);
}








static void node_sym (FILE* outfile, astree* node) {
    //fprintf (outfile, "%s (%s)", get_yytname (node->symbol), node->lexinfo->c_str());
    if(node->symbol == 0){
		fprintf (outfile, "%s", node->lexinfo->c_str());
    }else{
		fprintf (outfile, "%s (%s)", get_yytname (node->symbol), node->lexinfo->c_str());
    }
}


static void astree_rec_sym (FILE* outfile, astree* root, int depth) {
    if (root == NULL) return;
    fprintf (outfile, "%*s", depth * 4, "");
    dump_node (outfile, root);
    fprintf (outfile, "\n");
    for (size_t child = 0; child < root->children.size(); ++child) {
        dump_astree_rec (outfile, root->children[child], depth + 1);
    }
}

void astree_sym (SymbolTable* symTable, astree* root) {
    astree_rec_sym (symTable, root, 0);
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

RCSC("$Id: astree.cc,v 1.14 2013-10-10 18:48:18-07 - - $")

