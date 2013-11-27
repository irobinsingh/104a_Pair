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
#include "symtable.h"

SymbolTable *currentSymTable;

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

   fprintf(outfile, " chldrn: %d", node->children.size());
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL){
	   return;
   }else{
	   fprintf (outfile, "%*s", depth * 4, "");
	   dump_node (outfile, root);
	   fprintf (outfile, "\n");

	   for (size_t child = 0; child < root->children.size(); ++child) {
		   dump_astree_rec (outfile, root->children[child], depth + 1);
	   }
   }
}


void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}


static string nodeType(astree* node, SymbolTable* symTable) {
    
    if (node->symbol == TOK_IDENT) {
         return symTable->lookup(node->lexinfo->c_str());
        
    }
    else {
        
        return node->lexinfo->c_str();
    }
    
}

static void typeCheck (SymbolTable* symTable, astree* node) {
    switch (node->symbol)
    {
        case '=':
            if (node->children.size() != 2) {
                break;
            }
            if (strcmp(node->children[1]->lexinfo->c_str() , "binop") == 0) {
                printf(node->children[1]->lexinfo->c_str());
                
                if (strcmp( nodeType(node->children[1]->children[1]->children[0], symTable).c_str() , nodeType(node->children[1]->children[2]->children[0], symTable).c_str()  ) != 0) {
                    printf( "\nERROR: THE TYPES DONT MATCH: %s  %s\n", nodeType(node->children[1]->children[1]->children[0], symTable).c_str() , nodeType(node->children[1]->children[2]->children[0], symTable).c_str() );
                }
                
                break;
                
            }
            
            string s1 =  get_yytname (node->children[0]->symbol);
            string s2 =  get_yytname (node->children[1]->symbol);
          
            
            if (s2.find(s1) == std::string::npos) {
                printf( "\nERROR: THE TYPES DONT MATCH: %s  %s\n",node->children[0]->lexinfo->c_str() , node->children[1]->lexinfo->c_str() );
            }
            break;
            
        //default:
           // printf("\n");
    }

}

/*
static void typeCheck_astree_rec (astree* root, int depth) {
    if (root == NULL) return;
    typeCheck_node (root);
    for (size_t child = 0; child < root->children.size(); ++child) {
        typeCheck_astree_rec (root->children[child], depth + 1);
    }
}


void typeCheck_astree (astree* root) {
    typeCheck_astree_rec (root, 0);
    fflush (NULL);
}
 */

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

static int lexInfoToSwitch(const char* lexinfo){
	string struct_s = "structdef"; 		// 1
	
	string vardecl = "vardecl"; 		// 2
	string declaration = "declaration"; // 3
	
	string function = "function";		// 4
	
	string block = "block";				// 5
	string while_s = "while";			// 6
	string if_s = "if";					// 7
	string else_s = "else";				// 8
	
	int lexReturn = 0;
	
	if(strcmp(lexinfo, struct_s.c_str()) == 0){
		lexReturn = 1;
	}
	if(strcmp(lexinfo, vardecl.c_str()) == 0){
		lexReturn = 2;
	}
	if(strcmp(lexinfo, declaration.c_str()) == 0){
		lexReturn = 3;
	}
	if(strcmp(lexinfo, function.c_str()) == 0){
		lexReturn = 4;
	}
	if(strcmp(lexinfo, block.c_str()) == 0){
		lexReturn = 5;
	}
	if(strcmp(lexinfo, while_s.c_str()) == 0){
		lexReturn = 6;
	}
	if(strcmp(lexinfo, if_s.c_str()) == 0){
		lexReturn = 7;
	}
	if(strcmp(lexinfo, else_s.c_str()) == 0){
		lexReturn = 8;
	}
		
	return lexReturn;
}

static void astree_to_sym_rec (SymbolTable * symTable, astree* root) {
}

static void astree_to_sym_rec (astree* root) {
	if (root == NULL) return;

	switch(lexInfoToSwitch(root->lexinfo->c_str())){
		case 1:{//struct definition
			printf("****struct\n");
			SymbolTable *newStruct = new SymbolTable(NULL);
			struct_defs.push_back(newStruct);
			newStruct->addSymbol(root->children[0]->lexinfo->c_str(),"struct");
			newStruct = newStruct->enterBlock();
			astree_to_sym_rec(newStruct, root->children[1]);
			return;
			break;
		}
		case 2:{//variable declaration
			printf("current block number %d\n", currentSymTable->getNumber());
			if(root->children[0]->children[0]->children[0]->symbol == TOK_ARRAY){
					printf("%s _ %d,%d,%d _ %s%s\n", root->children[0]->children[0]->children[1]->lexinfo->c_str(), 
						root->filenr, root->linenr, root->offset, 
						root->children[0]->children[0]->lexinfo->c_str(),root->children[0]->children[0]->children[0]->lexinfo->c_str());
					
					string tempStr = root->children[0]->children[0]->lexinfo->c_str();
					tempStr.append(root->children[0]->children[0]->children[0]->lexinfo->c_str());
					
					currentSymTable->addSymbol(root->children[0]->children[0]->children[1]->lexinfo->c_str(), tempStr);
			}else{
				printf("%s _ %d,%d,%d _ %s\n", root->children[0]->children[0]->children[0]->lexinfo->c_str(),
					root->filenr, root->linenr, root->offset,
					root->children[0]->children[0]->lexinfo->c_str());
				
				currentSymTable->addSymbol(root->children[0]->children[0]->children[0]->lexinfo->c_str(),
						root->children[0]->children[0]->lexinfo->c_str());
			}
			break;
		}
		case 3:{//normal declaration
		
			printf("current block number %d\n", currentSymTable->getNumber());
			if(root->children[0]->children[0]->symbol == TOK_ARRAY){
				printf("%s _ %d,%d,%d _ %s%s\n", root->children[0]->children[1]->lexinfo->c_str(),
					root->filenr, root->linenr, root->offset, root->children[0]->lexinfo->c_str(),
					root->children[0]->children[0]->lexinfo->c_str());

				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append(root->children[0]->children[0]->lexinfo->c_str());

				currentSymTable->addSymbol(root->children[0]->children[1]->lexinfo->c_str(), tempStr);
			}else{
				printf("%s _ %d,%d,%d _ %s\n", root->children[0]->children[0]->lexinfo->c_str(),
					root->filenr, root->linenr, root->offset,
					root->children[0]->lexinfo->c_str());

				currentSymTable->addSymbol(root->children[0]->children[0]->lexinfo->c_str(),
						root->children[0]->lexinfo->c_str());
			}
			break;
		}
		case 4:{//function declaration
		
			printf("current block number %d\n", currentSymTable->getNumber());
			if(root->children[0]->children[0]->symbol == TOK_ARRAY){
				printf("%s _ %d,%d,%d _ %s(%s)\n\t", root->children[0]->children[1]->lexinfo->c_str(),
					root->filenr, root->linenr,
					root->offset, root->children[0]->lexinfo->c_str(),
					root->children[0]->children[0]->lexinfo->c_str());
					
				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append(root->children[0]->children[0]->lexinfo->c_str());

				currentSymTable = currentSymTable->enterFunction(root->children[0]->children[1]->lexinfo->c_str(), "(" + tempStr + ")" );
			}else{
				printf("%s(%s) _ %d,%d,%d _ %s\n\t", root->children[0]->children[0]->lexinfo->c_str(),
					root->children[0]->lexinfo->c_str(), root->filenr, root->linenr,
					root->offset, root->children[0]->children[1]->lexinfo->c_str());

				string tempStr = root->children[0]->children[0]->lexinfo->c_str();
				currentSymTable = currentSymTable->enterFunction(root->children[0]->children[1]->lexinfo->c_str(), "(" + tempStr + ")" );
			}
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
				
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 5:{//block
			printf("****entering block\n");
			currentSymTable = currentSymTable->enterBlock();
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
				
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 6:{//while loop
			printf("****entering while loop\n");
			currentSymTable = currentSymTable->enterBlock();
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
				
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 7:{//if block
			printf("****entering if block\n");
			currentSymTable = currentSymTable->enterBlock();
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
				
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 8:{//else block
			printf("****entering else block\n");
			currentSymTable = currentSymTable->enterBlock();
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
				
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 0:{//no matching condition. Do nothing. 
			break;
		}
	}

	for (size_t child = 0; child < root->children.size(); ++child) {
		astree_to_sym_rec (root->children[child]);
	}
}

void astree_to_sym (SymbolTable *symTable, astree* root) {
   currentSymTable = symTable;
   astree_to_sym_rec(root);
   fflush (NULL);
}

void type_check (SymbolTable *symTable, astree* node){
	char operaterer = node->symbol;
	
	/*
	switch(operaterer){
	default:
	break;
	'=':
	break;
	
	}*/
}

void dump_structs (FILE* output){
	for (size_t size = 0; size < struct_defs.size(); ++size) {
			struct_defs[size]->dump(output, 0);
		}
}
RCSC("$Id: astree.cc,v 1.14 2013-10-10 18:48:18-07 - - $")

