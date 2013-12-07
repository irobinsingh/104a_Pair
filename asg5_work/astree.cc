// Assignment 4 CS 104a 
// Modified By: Konstantin Litovskiy and Gahl Levy
// Users Names: klitovsk and grlevy

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream> 

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"
#include "symtable.h"

SymbolTable *currentSymTable;
SymbolTable *global_sym_table = new SymbolTable(NULL);
int global_temp_char_counter = 0;
int global_loop_counter = 0;

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

   fprintf(outfile, "\t--- chldrn: %d", node->children.size());
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

string typeCheck (astree* root, int counter) {
	if (root == NULL) return "";
	
	string returnString = "";
	
	if(root->children.size() == 0){
		switch(root->symbol){
			case TOK_INT:
				returnString = root->lexinfo->c_str();
				break;
			case TOK_CHAR:
				returnString = root->lexinfo->c_str();
				break;
			case TOK_STRING:
				returnString = root->lexinfo->c_str();
				break;
			case TOK_BOOL:
				returnString = root->lexinfo->c_str();
				break;
			case TOK_NULL:
				returnString = root->lexinfo->c_str();
				break;
			case TOK_VOID:{
				returnString = root->lexinfo->c_str();
				break;
			}
			case TOK_INTCON:{
				returnString = "int";
				break;
			}
			case TOK_CHARCON:{
				returnString = "char";
				break;
			}
			case TOK_STRINGCON:{
				returnString = "string";
				break;
			}
			case TOK_IDENT:{
				returnString = getIdentType(root->lexinfo->c_str(), counter);
				//printf("return string: %s\n", returnString.c_str());
				break;
			}
			default:{
				returnString = "";
				break;
			}
		}
	}else{
	
		string type1 = "";
		string type2 = "";
	
		//printf("switching on: %s\n", root->lexinfo->c_str());
		//printf("\tswitching on: %s\n", root->children[0]->lexinfo->c_str());
		switch(lexInfoToSwitch(root->lexinfo->c_str())){
			case 1:{//struct definition
			break;
			}
			case 2:{//variable declaration
				type1 = typeCheck(root->children[0]->children[0], counter);
				type2 = typeCheck(root->children[0]->children[1], counter);
				
				if(type1 != "" && type2 != ""){
					if(typeMatch(type1,type2)){
						return "";
					}else{
						errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s\n", 
							root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
					}
				}else{
					if(type1 == ""){
						errprintf ("***(%d,%d,%d) -- Unknown Ident: %s (type1)\n", root->filenr, root->linenr,root->offset, type1.c_str());
					}else{
						errprintf ("***(%d,%d,%d) -- Unknown Ident: %s (type2)\n", root->filenr, root->linenr,root->offset, type2.c_str());
					}
				}
				
			break;
			}
			case 3:{//normal declaration
			break;
			}
			case 4:{//function declaration
				counter++;
				break;
			}
			case 5:{//block
				counter++;
				break;
			}
			case 6:{//while loop
				counter++;
				break;
			}
			case 7:{//if block
				counter++;
				break;
			}
			case 8:{//else block
				counter++;
				break;
			}	
			case 10:{//binop typecheck
				type1 = typeCheck(root->children[0]->children[0], counter);
				type2 = typeCheck(root->children[0]->children[1], counter);
				
				//printf("type1: %s\n", type1.c_str());
				//printf("type2: %s\n", type2.c_str());
				
				if(type1 != "" && type2 != ""){
					switch(root->children[0]->symbol){
						case '=': 
						case TOK_EQ: 
						case TOK_NE:{
							if(typeMatch(type1,type2)){
								return type1;
							}else{
								errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s\n", 
									root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
							}
							break;
						}
						case '<': 
						case TOK_LE:
						case '>': 
						case TOK_GE:{
							if(typeMatch(type1,type2)){
								if(isNULL(type1) || isNULL(type2)){
									if(isNULL(type1)){
										if(isPrimative(type2)){
											return type2;
										}else{
											errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT, CHAR, or BOOL\n", 
												root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
										}										
									}else{
										if(isPrimative(type1)){
											return type1;
										}else{
											errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT, CHAR, or BOOL\n", 
												root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
										}
									}
								}else{
									if(isPrimative(type1)){
										return type1;
									}else{
										errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT, CHAR, or BOOL\n", 
											root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
									}
								}
							}else{
								errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT, CHAR, or BOOL\n", 
									root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
							}
							break;
						}
						case '+': 
						case '-': 
						case '*': 
						case '/': 
						case '%':{
							if(typeMatch(type1,type2)){
								if(isNULL(type1) || isNULL(type2)){
									if(isNULL(type1)){
										if(isInt(type2)){
											return type2;
										}else{
											errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT\n", 
												root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
										}										
									}else{
										if(isInt(type1)){
											return type1;
										}else{
											errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT\n", 
												root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
										}
									}
								}else{
									if(isInt(type1)){
										return type1;
									}else{
										errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT\n", 
											root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
									}
								}
							}else{
								errprintf ("***(%d,%d,%d) -- Miss-Matched Types: %s <-> %s | Can Only Be INT\n", 
									root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str(), type2.c_str());
							}					
							break;
						}	
					}
				}else{
					if(type1 == ""){
						errprintf ("***(%d,%d,%d) -- Unknown Ident: %s (type1)\n", root->filenr, root->linenr,root->offset, type1.c_str());
					}else{
						errprintf ("***(%d,%d,%d) -- Unknown Ident: %s (type2)\n", root->filenr, root->linenr,root->offset, type2.c_str());
					}
				}
				break;
			}
			case 11:{//unop typecheck
				type1 = typeCheck(root->children[0]->children[0], counter);
				
				if(type1 != ""){
					switch(root->children[0]->symbol){
						case '+': 
						case '-':{
							if(isInt(type1)){
								returnString =  type1;
							}else{
								errprintf ("***(%d,%d,%d) -- Miss-Matched Type: %s. MUST BE INT\n", 
									root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str());
							}	
							break;
						}
						case '!':{
							if(isBool(type1)){
								returnString =  type1;
							}else{
								errprintf ("***(%d,%d,%d) -- Miss-Matched Type: %s. MUST BE BOOL\n", 
									root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str());
							}
							break;
						}
						case TOK_ORD: {
							if(isChar(type1)){
								returnString =  "int";
							}else{
								errprintf ("***(%d,%d,%d) -- Miss-Matched Type: %s. MUST BE CHAR\n", 
									root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str());
							}
							break;
						}
						case TOK_CHAR:{
							if(isInt(type1)){
								returnString =  "char";
							}else{
								errprintf ("***(%d,%d,%d) -- Miss-Matched Type: %s. MUST BE INT\n", 
									root->children[0]->filenr, root->children[0]->linenr,root->children[0]->offset, type1.c_str());
							}
							break;
						}
					}
				}else{
					errprintf ("***(%d,%d,%d) -- Unknown Ident: %s\n", root->filenr, root->linenr,root->offset, type1.c_str());
				}
				break;	
			}
			case 12:{//variable typecheck
				if(root->children.size() == 2){
					type1 = typeCheck(root->children[0], counter);
					type2 = typeCheck(root->children[1], counter);
					string type2_name = root->children[1]->lexinfo->c_str();
					
					//	printf("type1: %s\n", type1.c_str());
					//	printf("type2_name: %s\n", type2_name.c_str());
					if (isStruct(type1) && isStructIdent(type1, type2_name)){
						returnString = getTypeFromStruct(type1, type2_name);
					//	printf("return from variable %s\n", returnString.c_str());
					}else{
						if(isString(type1) && isInt(type2)){
							returnString = "char";
						}else{
							if(isBaseTypeArr(type1) && isInt(type2)){
								returnString = type1.substr(0, type1.length()-2);
							}else{
								errprintf ("***(%d,%d,%d) -- Variable Miss Matched Type\n", root->filenr, root->linenr,root->offset);
							}
						}
					}
				}else{
					if(root->children.size() == 1){
						returnString = typeCheck(root->children[0], counter);
					}else{
						errprintf ("***(%d,%d,%d) -- Variable Argument Amount Wrong\n", root->filenr, root->linenr,root->offset);
					}
				}
				break;
			}
			case 13:{//new typecheck
				
				if(root->children.size() == 2){
					type1 = typeCheck(root->children[0], counter);
					type2 = typeCheck(root->children[1], counter);
					
					if(type1 != "" && type2 != ""){
						if(isBaseType(type1) && isInt(type2)){
							returnString = type1.append("[]");
						}else{
							errprintf ("***(%d,%d,%d) -- New Allocator Miss Matched Type\n", root->filenr, root->linenr,root->offset);
						}
					}else{
						if(type1 == ""){
							errprintf ("***(%d,%d,%d) -- Unknown Ident: %s\n", root->filenr, root->linenr,root->offset, type1.c_str());
						}else{
							errprintf ("***(%d,%d,%d) -- Unknown Ident: %s\n", root->filenr, root->linenr,root->offset, type2.c_str());
					}
				}
				}else{
					if(root->children.size() == 1){
						type1 = typeCheck(root->children[0], counter);
						if(type1 != ""){
							if(isBaseType(type1)){
								returnString = type1;
							}else{
								errprintf ("***(%d,%d,%d) -- New Allocator Miss Matched Type\n", root->filenr, root->linenr,root->offset);
							}
						}else{
							errprintf ("***(%d,%d,%d) -- Unknown Ident: %s\n", root->filenr, root->linenr,root->offset, type1.c_str());
						}	
					}else{
						errprintf ("***(%d,%d,%d) -- New Allocator Argument Amount Miss Matched Type\n", root->filenr, root->linenr,root->offset);
					}
				}		
				break;
			}
			case 14:{//call typecheck
				returnString = getFunctionType(root->children[0]->lexinfo->c_str(), counter);
				break;
			}
			case 0:{//no matching condition. Do nothing. 
				for (size_t child = 0; child < root->children.size(); ++child) {
					returnString = typeCheck (root->children[child], counter);
				}
				break;
			}
		}
	}
	return returnString;
}

string getTypeRec (astree* node){
	return "";
}

bool typeMatch (string type1, string type2){
	if(isNULL(type1) || isNULL(type2)){
		return true;
	}else{
		return (type1 == type2);
	}
}

bool isBaseType (string type){
	return isPrimative(type) || isString(type) || isStruct(type);
}

bool isBaseTypeArr (string type){
	string temp = type;
	if(type != "" && type.length() >2){
		temp = temp.substr(0,type.length()-2); 
		return isPrimative(temp) || isString(temp) || isStruct(temp);
	}else{
		return "";
	}
}

bool isPrimative (string type){
	return isInt(type) || isChar(type) || isBool(type); 
}

bool isInt (string type){
	string int_s = "int";
	return (strcmp(type.c_str(), int_s.c_str()) == 0);
}

bool isBool (string type){
	string bool_s = "bool";
	return (strcmp(type.c_str(), bool_s.c_str()) == 0);
}

bool isChar (string type){
	string char_s = "char";
	return (strcmp(type.c_str(), char_s.c_str()) == 0) ;
}

bool isString (string type){
	string string_s = "string";
	return (strcmp(type.c_str(), string_s.c_str()) == 0) ;
}

bool isNULL (string type){
	string null_s = "null";
	return (strcmp(type.c_str(), null_s.c_str()) == 0) ;
}

bool isStruct (string name){
	bool structFound = false;
	for(size_t tables = 0; tables < struct_defs.size(); ++tables){
		if(strcmp(struct_defs[tables]->lookup("struct").c_str(), name.c_str()) == 0){
			structFound = true;
			tables = struct_defs.size();
		}
	}
	return structFound;
}

bool isStructARR (string name){
	bool structFound = false;
	if(name.length() >= 2){
		string name_no_array = name.substr(0, name.length()-2);
		for(size_t tables = 0; tables < struct_defs.size(); ++tables){
			if(strcmp(struct_defs[tables]->lookup("struct").c_str(), name_no_array.c_str()) == 0){
				structFound = true;
				tables = struct_defs.size();
			}
		}
	}
	return structFound;
}

bool isStructIdent(string type1,string type2){
	bool identFound = false;
	bool structFound = false;
	size_t tables = 0;
	while(tables < struct_defs.size() && !identFound){
		if(strcmp(struct_defs[tables]->lookup("struct").c_str(), type1.c_str()) == 0){
			if(struct_defs[tables]->lookup(type2) != ""){
				identFound = true;
			}			
		}
		++tables;
	}
	return identFound;
}

string getIdentType(string name, int counter){
	//printf("name: %s\n", name.c_str());
	if(symbol_tables_tracker[counter] != NULL){
		string tableLookUp = symbol_tables_tracker[counter]->lookup(name);
		if (tableLookUp != ""){
			return tableLookUp;
		}else{
			//printf("name: %s\n", name.c_str());
			return getIdentInStruct(name);
		}
	}else{
		//printf("sym_table_null\n");
		return "";
	}
}

string getIdentInStruct(string name){
	string identType = "";
	for(size_t tables = 0; tables < struct_defs.size(); ++tables){
		if(struct_defs[tables]->lookup("struct").c_str()!= ""){
			identType = struct_defs[tables]->lookup("struct");
			tables = struct_defs.size();
		}
	}
	return identType;
}

string getTypeFromStruct(string struct_name, string name){
	string identType = "";
	int struct_number = -1;
	for(size_t tables = 0; tables < struct_defs.size(); ++tables){
		if(struct_defs[tables]->lookup("struct").c_str() == struct_name){
			struct_number = tables;
			tables = struct_defs.size();
		}
	}
	if(struct_number >= 0){
		return struct_defs[struct_number]->lookup(name);
	}else{
		return "";
	}
}

string getFunctionType(string name, int counter){
	if(symbol_tables_tracker[counter] != NULL){
		string tableLookUp = symbol_tables_tracker[counter]->lookup(name);
		if (tableLookUp != ""){
			int par = tableLookUp.find("(");
			return tableLookUp.substr(0,par).c_str();
		}else{
			//printf("name: %s\n", name.c_str());
			return tableLookUp;
		}
	}else{
		//printf("sym_table_null\n");
		return "";
	}
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

int lexInfoToSwitch(const char* lexinfo){
	string struct_s = "structdef"; 		// 1
	
	string vardecl = "vardecl"; 		// 2
	string declaration = "declaration"; // 3
	
	string function = "function";		// 4
	
	string block = "block";				// 5
	string while_s = "while";			// 6
	string if_s = "if";					// 7
	string else_s = "else";				// 8
	
	string binop = "binop"; 			// 10
	string unop  = "unop";				// 11
	string variable = "variable";		// 12
	string new_s = "new";				// 13
	string call = "call"; 				// 14
	
	
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
	if(strcmp(lexinfo, binop.c_str()) == 0){
		lexReturn = 10;
	}
	if(strcmp(lexinfo, unop.c_str()) == 0){
		lexReturn = 11;
	}
	if(strcmp(lexinfo, variable.c_str()) == 0){
		lexReturn = 12;
	}
	if(strcmp(lexinfo, new_s.c_str()) == 0){
		lexReturn = 13;
	}
	if(strcmp(lexinfo, call.c_str()) == 0){
		lexReturn = 14;
	}
		
	return lexReturn;
}

string typeToOil(const char* type){
	string bool_s = "bool"; 			// ubyte
	string char_s = "char"; 			// ubyte
	string int_s = "int"; 				// int
	string string_s = "string";			// ubute*
	string boolARR_s = "bool[]";		// ubyte*
	string charARR_s = "char[]";		// ubyte*
	string intARR_s = "int[]";			// int*
	string stringARR_s = "string[]";	// ubyte**
		
	string return_type = "";
	
	if(type == bool_s){
		return_type = "ubyte ";
	}else if(type == char_s){
		return_type = "ubyte ";
	}else if(type == int_s){
		return_type = "int ";
	}else if(type == string_s){
		return_type = "ubyte *";
	}else if(type == boolARR_s){
		return_type = "ubyte *";
	}else if(type == charARR_s){
		return_type = "ubyte *";
	}else if(type == intARR_s){
		return_type = "int *";
	}else if(type == stringARR_s){
		return_type = "ubyte **";
	}else if(isStruct(type)){
		return_type = "struct ";
		return_type.append(type);
		return_type.append(" *");
	}else if(isStructARR(type)){
		string type_s = type;
		return_type = "struct ";
		return_type.append(type_s.substr(0, type_s.length()-2));
		return_type.append( " **");
	}
		
	return return_type;
}

static void astree_to_sym_rec (SymbolTable * symTable, astree* root) {
	if (root == NULL) return;
	
	switch(lexInfoToSwitch(root->lexinfo->c_str())){
		case 1:{//struct definition
			break;
		}
		case 2:{//variable declaration
			//printf("current block number %d\n", symTable->getNumber());
			if(root->children[0]->children[0]->children[0]->symbol == TOK_ARRAY){
					//printf("%s _ %d,%d,%d _ %s%s\n", root->children[0]->children[0]->children[1]->lexinfo->c_str(), 
					//	root->filenr, root->linenr, root->offset, 
					//	root->children[0]->children[0]->lexinfo->c_str(),root->children[0]->children[0]->children[0]->lexinfo->c_str());
					
					string tempStr = root->children[0]->children[0]->lexinfo->c_str();
					tempStr.append(root->children[0]->children[0]->children[0]->lexinfo->c_str());
					
					symTable->addSymbol(root->children[0]->children[0]->children[1]->lexinfo->c_str(), 
												tempStr,
												root->children[0]->children[0]->children[1]->filenr,
												root->children[0]->children[0]->children[1]->linenr, 
												root->children[0]->children[0]->children[1]->offset);
			}else{
				//printf("%s _ %d,%d,%d _ %s\n", root->children[0]->children[0]->children[0]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset,
				//	root->children[0]->children[0]->lexinfo->c_str());
				
				symTable->addSymbol(root->children[0]->children[0]->children[0]->lexinfo->c_str(),
											root->children[0]->children[0]->lexinfo->c_str(),
											root->children[0]->children[0]->children[0]->filenr,
											root->children[0]->children[0]->children[0]->linenr, 
											root->children[0]->children[0]->children[0]->offset);
			}
			break;
		}
		case 3:{//normal declaration
		
			//printf("current block number %d\n", symTable->getNumber());
			if(root->children[0]->children[0]->symbol == TOK_ARRAY){
				//printf("%s _ %d,%d,%d _ %s%s\n", root->children[0]->children[1]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset, root->children[0]->lexinfo->c_str(),
				//	root->children[0]->children[0]->lexinfo->c_str());

				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append(root->children[0]->children[0]->lexinfo->c_str());

				symTable->addSymbol(root->children[0]->children[1]->lexinfo->c_str(),
											tempStr,
											root->children[0]->children[1]->filenr,
											root->children[0]->children[1]->linenr,
											root->children[0]->children[1]->offset);
			}else{
				//printf("%s _ %d,%d,%d _ %s\n", root->children[0]->children[0]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset,
				//	root->children[0]->lexinfo->c_str());

				symTable->addSymbol(root->children[0]->children[0]->lexinfo->c_str(),
											root->children[0]->lexinfo->c_str(),
											root->children[0]->children[0]->filenr,
											root->children[0]->children[0]->linenr,
											root->children[0]->children[0]->offset);
			}
			break;
		}
		case 4:{//function declaration
			bool arr = false;
			int dec = 0;
			int block = 0;
			astree* tempPtr;
			
			//printf("current block number %d\n", symTable->getNumber());
			if(root->children[0]->children[0]->symbol == TOK_ARRAY){
				arr = true;
				//printf("%s _ %d,%d,%d _ %s()\n\t", root->children[0]->children[1]->lexinfo->c_str(),
				//	root->filenr, root->linenr,
				//	root->offset, root->children[0]->lexinfo->c_str());
				
				if(strcmp(root->children[0]->children[2]->lexinfo->c_str(), "block") == 0){
					block = 2;
				}else{
					dec = 2;
					block = 3;
				}
				
				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append(root->children[0]->children[0]->lexinfo->c_str());
				
				tempStr.append("(");
				
				if(dec > 0){
					tempPtr = root->children[0]->children[dec];
										
					for(size_t child = 0; child < tempPtr->children.size(); ++child){
						if(child != 0){
							tempStr.append(",");
						}
						tempStr.append(tempPtr->children[child]->children[0]->lexinfo->c_str());
					}
				}
				
				tempStr.append(")");
				//printf("signature %s\n",tempStr.c_str());
				
				symTable = symTable->enterFunction(root->children[0]->children[1]->lexinfo->c_str(),
																	tempStr,
																	root->children[0]->children[1]->filenr,
																	root->children[0]->children[1]->linenr,
																	root->children[0]->children[1]->offset);
			}else{
				//printf("%s _ %d,%d,%d _ %s()\n\t", root->children[0]->children[0]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset, 
				//	root->children[0]->lexinfo->c_str());

				if(strcmp(root->children[0]->children[1]->lexinfo->c_str(), "block") == 0){
					block = 1;
				}else{
					dec = 1;
					block = 2;
				}
				
				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append("(");
				
				if(dec > 0){
					tempPtr = root->children[0]->children[dec];
										
					for(size_t child = 0; child < tempPtr->children.size(); ++child){
						if(child != 0){
							tempStr.append(",");
						}
						tempStr.append(tempPtr->children[child]->children[0]->lexinfo->c_str());
					}
				}
				
				tempStr.append(")");
				//printf("signature %s\n",tempStr.c_str());
				
				symTable = symTable->enterFunction(root->children[0]->children[0]->lexinfo->c_str(),
																	tempStr,
																	root->children[0]->children[0]->filenr,
																	root->children[0]->children[0]->linenr,
																	root->children[0]->children[0]->offset);
			}
			
			if(dec > 0){
				for (size_t child = 0; child < root->children[0]->children[dec]->children.size(); ++child) {
					astree_to_sym_rec (symTable, root->children[0]->children[dec]->children[child]);
				}
			}
			if(block > 0){
				for (size_t child = 0; child < root->children[0]->children[block]->children.size(); ++child) {
					astree_to_sym_rec (symTable, root->children[0]->children[block]->children[child]);
				}
			}
				
			symTable = symTable->getParent();
			
			//global_sym_table->dump(stdout, 0);
			
			break;
		}
		case 5:{//block
		}
		case 6:{//while loop
			break;
		}
		case 7:{//if block
			break;
		}
		case 8:{//else block
			break;
		}
		case 0:{//no matching condition. Do nothing. 
			break;
		}
	}

	for (size_t child = 0; child < root->children.size(); ++child) {
		astree_to_sym_rec (symTable, root->children[child]);
	}
}

static void astree_to_sym_rec (astree* root) {
	if (root == NULL) return;
	
	switch(lexInfoToSwitch(root->lexinfo->c_str())){
		case 1:{//struct definition
			//printf("****struct\n");
			SymbolTable *newStruct = new SymbolTable(NULL);
			struct_defs.push_back(newStruct);
			newStruct->addSymbol("struct",
									root->children[0]->lexinfo->c_str(),
									root->children[0]->filenr,
									root->children[0]->linenr,
									root->children[0]->offset);
			astree_to_sym_rec(newStruct, root->children[1]);
			return;
			break;
		}
		case 2:{//variable declaration
			//printf("current block number %d\n", currentSymTable->getNumber());
			if(root->children[0]->children[0]->children[0]->symbol == TOK_ARRAY){
					//printf("%s _ %d,%d,%d _ %s%s\n", root->children[0]->children[0]->children[1]->lexinfo->c_str(), 
					//	root->filenr, root->linenr, root->offset, 
					//	root->children[0]->children[0]->lexinfo->c_str(),root->children[0]->children[0]->children[0]->lexinfo->c_str());
					
					string tempStr = root->children[0]->children[0]->lexinfo->c_str();
					tempStr.append(root->children[0]->children[0]->children[0]->lexinfo->c_str());
					
					currentSymTable->addSymbol(root->children[0]->children[0]->children[1]->lexinfo->c_str(), 
												tempStr,
												root->children[0]->children[0]->children[1]->filenr,
												root->children[0]->children[0]->children[1]->linenr, 
												root->children[0]->children[0]->children[1]->offset);
			}else{
				//printf("%s _ %d,%d,%d _ %s\n", root->children[0]->children[0]->children[0]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset,
				//	root->children[0]->children[0]->lexinfo->c_str());
				
				currentSymTable->addSymbol(root->children[0]->children[0]->children[0]->lexinfo->c_str(),
											root->children[0]->children[0]->lexinfo->c_str(),
											root->children[0]->children[0]->children[0]->filenr,
											root->children[0]->children[0]->children[0]->linenr, 
											root->children[0]->children[0]->children[0]->offset);
			}
			break;
		}
		case 3:{//normal declaration
		
			//printf("current block number %d\n", currentSymTable->getNumber());
			if(root->children[0]->children[0]->symbol == TOK_ARRAY){
				//printf("%s _ %d,%d,%d _ %s%s\n", root->children[0]->children[1]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset, root->children[0]->lexinfo->c_str(),
				//	root->children[0]->children[0]->lexinfo->c_str());

				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append(root->children[0]->children[0]->lexinfo->c_str());

				currentSymTable->addSymbol(root->children[0]->children[1]->lexinfo->c_str(),
											tempStr,
											root->children[0]->children[1]->filenr,
											root->children[0]->children[1]->linenr,
											root->children[0]->children[1]->offset);
			}else{
				//printf("%s _ %d,%d,%d _ %s\n", root->children[0]->children[0]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset,
				//	root->children[0]->lexinfo->c_str());

				currentSymTable->addSymbol(root->children[0]->children[0]->lexinfo->c_str(),
											root->children[0]->lexinfo->c_str(),
											root->children[0]->children[0]->filenr,
											root->children[0]->children[0]->linenr,
											root->children[0]->children[0]->offset);
			}
			break;
		}
		case 4:{//function declaration
			bool arr = false;
			int dec = 0;
			int block = 0;
			astree* tempPtr;
			
			//printf("current block number %d\n", currentSymTable->getNumber());
			if(root->children[0]->children[0]->symbol == TOK_ARRAY){
				arr = true;
				//printf("%s _ %d,%d,%d _ %s()\n\t", root->children[0]->children[1]->lexinfo->c_str(),
				//	root->filenr, root->linenr,
				//	root->offset, root->children[0]->lexinfo->c_str());
				
				if(strcmp(root->children[0]->children[2]->lexinfo->c_str(), "block") == 0){
					block = 2;
				}else{
					dec = 2;
					block = 3;
				}
				
				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append(root->children[0]->children[0]->lexinfo->c_str());
				
				tempStr.append("(");
				
				if(dec > 0){
					tempPtr = root->children[0]->children[dec];
										
					for(size_t child = 0; child < tempPtr->children.size(); ++child){
						if(child != 0){
							tempStr.append(",");
						}
						tempStr.append(tempPtr->children[child]->children[0]->lexinfo->c_str());
					}
				}
				
				tempStr.append(")");
				//printf("signature %s\n",tempStr.c_str());
				
				currentSymTable = currentSymTable->enterFunction(root->children[0]->children[1]->lexinfo->c_str(),
																	tempStr,
																	root->children[0]->children[1]->filenr,
																	root->children[0]->children[1]->linenr,
																	root->children[0]->children[1]->offset);
				symbol_tables_tracker.push_back(currentSymTable);
			}else{
				//printf("%s _ %d,%d,%d _ %s()\n\t", root->children[0]->children[0]->lexinfo->c_str(),
				//	root->filenr, root->linenr, root->offset, 
				//	root->children[0]->lexinfo->c_str());

				if(strcmp(root->children[0]->children[1]->lexinfo->c_str(), "block") == 0){
					block = 1;
				}else{
					dec = 1;
					block = 2;
				}
				
				string tempStr = root->children[0]->lexinfo->c_str();
				tempStr.append("(");
				
				if(dec > 0){
					tempPtr = root->children[0]->children[dec];
										
					for(size_t child = 0; child < tempPtr->children.size(); ++child){
						if(child != 0){
							tempStr.append(",");
						}
						tempStr.append(tempPtr->children[child]->children[0]->lexinfo->c_str());
					}
				}
				
				tempStr.append(")");
				//printf("signature %s\n",tempStr.c_str());
				
				currentSymTable = currentSymTable->enterFunction(root->children[0]->children[0]->lexinfo->c_str(),
																	tempStr,
																	root->children[0]->children[0]->filenr,
																	root->children[0]->children[0]->linenr,
																	root->children[0]->children[0]->offset);
				symbol_tables_tracker.push_back(currentSymTable);
			}
			
			if(dec > 0){
				for (size_t child = 0; child < root->children[0]->children[dec]->children.size(); ++child) {
					astree_to_sym_rec (root->children[0]->children[dec]->children[child]);
				}
			}
			if(block > 0){
				for (size_t child = 0; child < root->children[0]->children[block]->children.size(); ++child) {
					astree_to_sym_rec (root->children[0]->children[block]->children[child]);
				}
			}
			
			currentSymTable = currentSymTable->getParent();
			
			//global_sym_table->dump(stdout, 0);
			
			break;
		}
		case 5:{//block
			//printf("****entering block\n");
			currentSymTable = currentSymTable->enterBlock();
			symbol_tables_tracker.push_back(currentSymTable);
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
			
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 6:{//while loop
			//printf("****entering while loop\n");
			currentSymTable = currentSymTable->enterBlock();
			symbol_tables_tracker.push_back(currentSymTable);
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
				
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 7:{//if block
			//printf("****entering if block\n");
			currentSymTable = currentSymTable->enterBlock();
			symbol_tables_tracker.push_back(currentSymTable);
			
				for (size_t child = 0; child < root->children.size(); ++child) {
					astree_to_sym_rec (root->children[child]);
				}
				
			currentSymTable = currentSymTable->getParent();
			break;
		}
		case 8:{//else block
			//printf("****entering else block\n");
			currentSymTable = currentSymTable->enterBlock();
			symbol_tables_tracker.push_back(currentSymTable);
			
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

void astree_to_sym (astree* root) {
   currentSymTable = global_sym_table;
   astree_to_sym_rec(root);
   fflush (NULL);
}
/*
string getNewTempVar(string ident){
	string return_string = ident;
	ident.append(++global_temp_char_counter;
}*/

static string astree_to_oil_rec (FILE* output, astree* root, int counter, int indent, bool inStruct){
	if (root == NULL) return "";
	
	string return_string = "";
	
	int indent_count = 0;
	//string indent_print = "_";
	
	if(root->children.size() == 0){
		switch(root->symbol){
			case TOK_INT:
				break;
			case TOK_CHAR:
				break;
			case TOK_STRING:
				break;
			case TOK_BOOL:
				break;
			case TOK_NULL:
				break;
			case TOK_VOID:{
				break;
			}
			case TOK_INTCON:{
				return_string = root->lexinfo->c_str();
				break;
			}
			case TOK_CHARCON:{
				return_string = root->lexinfo->c_str();
				break;
			}
			case TOK_STRINGCON:{
				return_string = root->lexinfo->c_str();
				break;
			}
			case TOK_IDENT:{
				return_string = root->lexinfo->c_str();
				break;
			}
			default:{
				break;
			}
		}
	}else{
	
		//printf("switching on: %s\n", root->lexinfo->c_str());
		//printf("\tswitching on: %s\n", root->children[0]->lexinfo->c_str());
		switch(lexInfoToSwitch(root->lexinfo->c_str())){
			case 1:{//struct definition
				fprintf(output, "struct %s {\n", root->children[0]->lexinfo->c_str());
				astree_to_oil_rec(output, root->children[1], counter, indent+1, true);
				fprintf(output, "};\n\n");
				break;
			}
			case 2:{//variable
				string type = "";
				string name = "";
				string right_hand = "";
				
				if(root->children[0]->children[0]->children[0]->symbol == TOK_ARRAY){
					name = root->children[0]->children[0]->children[1]->lexinfo->c_str();
					type = root->children[0]->children[0]->lexinfo->c_str();
					type.append(root->children[0]->children[0]->children[0]->lexinfo->c_str());
					type = typeToOil(type.c_str());
					right_hand = astree_to_oil_rec(output, root->children[0]->children[1], counter, indent, inStruct);
				}else{
					name = root->children[0]->children[0]->children[0]->lexinfo->c_str();
					type = typeToOil(root->children[0]->children[0]->lexinfo->c_str());
					right_hand = astree_to_oil_rec(output, root->children[0]->children[1], counter, indent, inStruct);
				}
				
				if(indent >= 1){
					fprintf(output,"%*s%s_%d_%s = %s;\n", indent * 8, "",
						type.c_str(),
						indent,
						name.c_str(),
						right_hand.c_str());
				}else{
					fprintf(output,"%*s%s__%s = %s;\n", indent * 8, "",
						type.c_str(),
						name.c_str(),
						right_hand.c_str());
				}
				break;
			}
			case 3:{//normal declaration
				string type = "";
				string name = "";
				
				if(root->children[0]->children[0]->symbol == TOK_ARRAY){
					name = root->children[0]->children[1]->lexinfo->c_str();
					type = root->children[0]->lexinfo->c_str();
					type.append(root->children[0]->children[0]->lexinfo->c_str());
					type = typeToOil(type.c_str());
				}else{
					name = root->children[0]->children[0]->lexinfo->c_str();
					type = typeToOil(root->children[0]->lexinfo->c_str());
				}
				
				if(inStruct){
					fprintf(output,"%*s%s%s;\n", indent * 8, "",
						type.c_str(),
						name.c_str());
				}else{
					if(indent >= 1){
						fprintf(output,"%*s%s_%d_%s;\n", indent * 8, "",
							type.c_str(),
							indent,
							name.c_str());
					}else{
						fprintf(output,"%*s%s__%s;\n", indent * 8, "",
							type.c_str(),
							name.c_str());
					}
				}
				break;
			}
			case 4:{//function declaration
				counter++;
				
				break;
			}
			case 5:{//block
				counter++;
				break;
			}
			case 6:{//while loop
				counter++;
				break;
			}
			case 7:{//if block
				counter++;
				break;
			}
			case 8:{//else block
				counter++;
				break;
			}	
			case 10:{//binop 
			string right = "";
			string left = "";
					left = astree_to_oil_rec(output, 
												root->children[0]->children[0],
												counter, indent, inStruct); 
					right = astree_to_oil_rec(output, 
											root->children[0]->children[1], 
											counter, indent, inStruct);
				if(root->children[0]->symbol != '='){
					stringstream temp_var;
					temp_var << "i" << ++global_temp_char_counter;
					string temp_var_s = temp_var.str();
			
					fprintf (output,"%*s%s = %s %s %s;\n", indent * 8, "",
						temp_var_s.c_str(),
						left.c_str(),
						root->children[0]->lexinfo->c_str(),
						right.c_str());
					return_string = temp_var.str();
				}else{
					fprintf (output,"%*s%s = %s;\n", indent * 8, "",
						left.c_str(),
						right.c_str());
				}
				break;	
			}
			case 12:{//variable 
				if(root->children.size() == 2){
					string right = astree_to_oil_rec(output, 
													root->children[0],
													counter, indent, inStruct); 
					string left = astree_to_oil_rec(output, 
													root->children[1],
													counter, indent, inStruct);
					
					if(isBaseTypeArr(typeCheck(root->children[0], counter))){
						return_string = right + "[ " + left + " ]";
					}else{
						return_string = right + "->" + left;
					}
				}else{
					return_string = astree_to_oil_rec(output, 
													root->children[0],
													counter, indent, inStruct);
				}
				break;
			}
			case 13:{//new 
				string expresion = "";
				if(root->children.size() == 2){
					string expression = astree_to_oil_rec(output, root->children[1], counter, indent, inStruct);
					
					if(isStruct(root->children[0]->lexinfo->c_str())){
						return_string = "xcalloc(";
						return_string += expression.c_str(); 
						return_string += ", sizeof(";
						return_string += root->children[0]->lexinfo->c_str();
						return_string += "));\n";
					}else{
						if(isInt(root->children[0]->lexinfo->c_str())){
							return_string = "xcalloc(";
							return_string += expression.c_str(); 
							return_string += ", sizeof(int));\n";
						}else{
							return_string = "xcalloc(";
							return_string += expression.c_str(); 
							return_string += ", sizeof(ubyte));\n";
						}
					}
				}else{
					if(isStruct(root->children[0]->lexinfo->c_str())){
						return_string = "xcalloc(1, sizeof(";
						return_string += root->children[0]->lexinfo->c_str();
						return_string += "));\n";
					}else{
						if(isInt(root->children[0]->lexinfo->c_str())){
							return_string = "xcalloc(1, sizeof(int));\n";
						}else{
							return_string = "xcalloc(1, sizeof(ubyte));\n";
						}
					}
				}		
				break;
			}
			case 14:{//call 
				fprintf(output,"__%s (%s);\n", root->children[0]->lexinfo->c_str(), root->children[0]->children[0]->lexinfo->c_str());
				break;
			}
			case 0:{//no matching condition. Do nothing. 
				for (size_t child = 0; child < root->children.size(); ++child) {
					//returnString = typeCheck (root->children[child], counter);
					astree_to_oil_rec (output, root->children[child], counter, indent, inStruct);
				}
				break;
			}
		}
	}
	
	return return_string;
}

void astree_to_oil (FILE* output, astree* root){
	fprintf(output,"#define __OCLIB_C__\n");
	fprintf(output,"#include \"oclib.oh\"\n\n");
	fprintf(output,"void __ocmain ()\n{\n");
	astree_to_oil_rec(output, root, 0, 0, false);
	fprintf(output,"}\n");
}

/*
void type_check (SymbolTable *symTable, astree* node){
	char operaterer = node->symbol;
	
	/*
	switch(operaterer){
	default:
	break;
	'=':
	break;
	
	}
}
*/

void dump_structs (FILE* output){
	for (size_t size = 0; size < struct_defs.size(); ++size) {
			fprintf(output, "%s:\n", struct_defs[size]->lookup("struct").c_str());
			struct_defs[size]->dump(output, 0);
		}
}
RCSC("$Id: astree.cc,v 1.14 2013-10-10 18:48:18-07 - - $")


