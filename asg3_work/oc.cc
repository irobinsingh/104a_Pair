// $Id: cppstrtok.cc,v 1.2 2013-09-20 19:38:26-07 - - $

// Assignment 2 CS 104a 
// Authors: Konstantin Litovskiy and Gahl Levy
// Users Names: klitovsk and grlevy

#include <string>
using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>

#include "auxlib.h"
#include "astree.h"
#include "stringset.h"
#include "lyutils.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;
FILE* tok_file_out = NULL;
extern int yy_flex_debug;
extern int yydebug;

int main (int argc, char **argv) {

    int arg;
    string input_file = "";
    string debugFlag = "";
    string baseName = "";
    string programName = "";
    string cppInput = "";
    char * fileName;
    yy_flex_debug = 0;
    yydebug = 0;
    
    //uses getopt to parse the command line arguments
    while((arg =  getopt(argc, argv, "ly@:D:")) != -1){ 
        switch (arg){
            case 'l':
                yy_flex_debug = 1;
                break;
            case 'y':
				yydebug = 1;
                break; 
            case '@':
                debugFlag = optarg;
                set_debugflags(debugFlag.c_str());
                break;
            case 'D':
                cppInput = optarg;
                break;
            case ':':
                // outputs error message to standard error
                syserrprintf ("Requires Input"); 
                set_exitstatus (1);
                break;
            case '?':
                syserrprintf ("not a valid argument");
                set_exitstatus (1);
                break;
            }
        }
        if(get_exitstatus() != 0){
            syserrprintf ("Invalid argument");

        }else{
            input_file = argv[optind];

            // checks for the correct file extension
            if(input_file.compare(input_file.length()-2,
                                      input_file.length(),"oc")!= 0){

                syserrprintf ("Unknown file extension");
                set_exitstatus (1);
            }else{ // if the correct file extension was found
                fileName = argv[optind];
                baseName = basename(fileName);
                programName = baseName.substr(0, baseName.length()-3);
            }
        }

        if(get_exitstatus() == 0){
            
            string command = CPP + " ";
            
            if (cppInput.compare("") != 0){
               command += "-D " + cppInput + " ";
            }
            
            set_execname(fileName);
            
            command += fileName;
            yyin = popen (command.c_str(), "r"); // opens the pipe
            if (yyin == NULL) {
            syserrprintf (command.c_str());
        }else {

            try {
                string outputFileName = programName + ".tok";
                tok_file_out = fopen (outputFileName.c_str(),"w");
                
				yyparse();
				
                fclose (tok_file_out); // close the str file

            } catch (...) { // if there is an error with the file
                syserrprintf ("File Error");
        }



            string outputFileNameSTR = programName + ".str";
			string outputFileNameAST = programName + ".ast";
			
            try {
				
                FILE *outputFileSTR = fopen (outputFileNameSTR.c_str(),"w");

                // writes the strings to the file
                dump_stringset (outputFileSTR); 
                fclose (outputFileSTR); // close the str file
            } catch (...) { // if there is an error with the file
               string errout = "File Error: Failed to write to " + outputFileNameSTR + ".";
               syserrprintf (errout.c_str());
            }
			
			try {
				
                FILE *outputFileAST = fopen (outputFileNameAST.c_str(),"w");

                // writes the strings to the file
                dump_astree (outputFileAST, yyparse_astree);
                fclose (outputFileAST); // close the str file

            } catch (...) { // if there is an error with the file
                string errout2 = "File Error: Failed to write to " + outputFileNameAST + ".";
                syserrprintf (errout2.c_str());
            }

        }
    }
    else{
        syserrprintf ("Invalid Arguments");
    }
    return get_exitstatus();
}

