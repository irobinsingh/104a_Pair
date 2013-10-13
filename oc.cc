// $Id: cppstrtok.cc,v 1.2 2013-09-20 19:38:26-07 - - $

// Assignment 1 CS 104a 
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
#include "stringset.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

// Chomp the last character from a buffer if it is delim.
void chomp (char *string, char delim) {
   size_t len = strlen (string); //gets the length of the string
   if (len == 0) return;
   char *nlpos = string + len - 1; //removes last character
   if (*nlpos == delim) *nlpos = '\0';
}

// Run cpp against the lines of the file.
void cpplines (FILE *pipe, char *filename) {

	// printf("in cpplines\n");
   char inputname[LINESIZE];
   strcpy (inputname, filename);
	for (;;) {
		char buffer[LINESIZE];
		char *fgets_rc = fgets (buffer, LINESIZE, pipe); // pipe
		if (fgets_rc == NULL) break;
		chomp (buffer, '\n');
	char *savepos = NULL;
	char *bufptr = buffer;

	for (int tokenct = 1;; ++tokenct) { // this section tokenizes the string
		char *token = strtok_r (bufptr, " \t\n", &savepos); // tells the tokenizer what to look for
		bufptr = NULL;
		if (token == NULL) break;
		intern_stringset (token);  //inserts into the hashtable
	}
   }
}

int main (int argc, char **argv) {

	int arg;
	string input_file = "";
	string debugFlag = "";
	string baseName = "";
	string programName = "";
	char * fileName;
	while((arg =  getopt(argc, argv, "ly@:D:")) != -1){ //uses getopt to parse the command line arguments
		switch (arg){
		case 'l':
		   break;
		 case 'y':
			break; // these flags will be used in future assignments
		 case '@':
			 debugFlag = optarg;
			 set_debugflags(debugFlag.c_str());
			break;
		 case 'D':
			break;
		case ':':
			syserrprintf ("Requires Input"); // outputs error message to standard error
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
		//printf ("input file: %s\n", argv[optind]);
		input_file = argv[optind];

		if(input_file.compare(input_file.length()-2, input_file.length(), "oc") != 0 ){ // checks for the correct file extension
			syserrprintf ("Unknown file extension");
			set_exitstatus (1);
		}else{ // if the correct file extension was found
			fileName = argv[optind];
			baseName = basename(fileName);
			programName = baseName.substr(0, baseName.length()-3);
			//printf("\"baseName\" %s\n\"programName\" %s\n", baseName.c_str(), programName.c_str());
		}
	}

	if(get_exitstatus() == 0){
		//printf("strcpy: %s\n", fileName);
		string command = CPP + " " + fileName;
		//printf ("command=\"%s\"\n", command.c_str());
		FILE *pipe = popen (command.c_str(), "r"); // opens the pipe
		if (pipe == NULL) {
			syserrprintf (command.c_str());
		}else {
			cpplines (pipe, fileName);
			int pclose_rc = pclose (pipe); // closes the pipe 
			eprint_status (command.c_str(), pclose_rc);
            
            try {
                string outputFileName = programName + ".str";
                
                FILE *outputFile = fopen (outputFileName.c_str(),"w");
                
                dump_stringset (outputFile); // writes the strings to the file
                fclose (outputFile); // close the str file
                
            } catch (...) { // if there is an error with the file
                syserrprintf ("File Error");
            }

			
		}
	}
    else{
        syserrprintf ("Invalid Arguments");
    }
	return get_exitstatus();
}

