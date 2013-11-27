#ifndef __SYMDATA_H__
#define __SYMDATA_H__

#include <stdio.h>

using namespace std;

class SymData {
	
	int symbol_file_number;
	int symbol_line_number;
	int symbol_offset;
	string symbol_type;

public:
  SymData(string type, int file, int line, int offset){
	symbol_file_number = file;
	symbol_line_number = line;
	symbol_offset = offset;
	symbol_type = type;}
	
	int get_file_number(){ return symbol_file_number;}
	int get_line_number(){ return symbol_line_number;}
	int get_offset(){ return symbol_offset;}
	string get_type(){ return symbol_type;}
};

#endif
