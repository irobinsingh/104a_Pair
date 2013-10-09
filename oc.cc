// $Id: cppstrtok.cc,v 1.2 2013-09-20 19:38:26-07 - - $

// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

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
   size_t len = strlen (string);
   if (len == 0) return;
   char *nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

// Run cpp against the lines of the file.
void cpplines (FILE *pipe, char *filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char *fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
     /* 			  printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
					  // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
					  int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
											  &linenr, filename);
					  if (sscanf_rc == 2) {
						 printf ("DIRECTIVE: line %d file \"%s\"\n", linenr, filename);
						 continue;
					  }
					  char *savepos = NULL;
					  char *bufptr = buffer;
					  for (int tokenct = 1;; ++tokenct) {
						 char *token = strtok_r (bufptr, " \t\n", &savepos);
						 bufptr = NULL;
						 if (token == NULL) break;
						 printf ("token %d.%d: [%s]\n",
								 linenr, tokenct, token);
					  }*/
      intern_stringset (fgets_rc);
      ++linenr;
   }
}

int main (int argc, char **argv) {

	int arg;
	int arg_error = 0;
	int oc_error = 0;
	string input_file = "";
	string debugFlag = "";
	string baseName = "";
	char *filename;
	while((arg =  getopt(argc, argv, "ly@:D:")) != -1){
		switch (arg){
		case 'l':
		   printf("%s\n", "-l");
		   break;
		 case 'y':
			 printf("%s\n", "-y");
			break;
		 case '@':
			 debugFlag = optarg;
			 printf("flag: %s\n",debugFlag.c_str());
			 set_debugflags(debugFlag.c_str());
			break;
		 case 'D':
			 printf("file: %s\n",optarg);
			break;
		case ':':
			syserrprintf ("Requires Input");
			arg_error = 1;
			break;
		case '?':
			syserrprintf (" is not a valid argument");
			arg_error = 1;
			break;
		}
	}
	if(arg_error != 0){
		printf ("error\n");

	}else{
		printf ("input file: %s\n", argv[optind]);
		input_file = argv[optind];

		if(input_file.compare(input_file.length()-2, input_file.length(), "oc") != 0 ){
			printf ("unknonw file extention\n");
			oc_error = 1;
		}else{
			filename = argv[optind];
			baseName = basename(filename);
			printf("\"filename\" %s\n", baseName.c_str());
		}
	}

	if(oc_error == 0){
		string command = CPP + " " + filename;
		printf ("command=\"%s\"\n", command.c_str());
		FILE *pipe = popen (command.c_str(), "r");
		if (pipe == NULL) {
			syserrprintf (command.c_str());
		}else {
			cpplines (pipe, filename);
			int pclose_rc = pclose (pipe);
			eprint_status (command.c_str(), pclose_rc);

			string outputFileName = baseName + ".str";
			FILE *outputFile = fopen (outputFileName.c_str(),"w");

			dump_stringset (outputFile);
			fclose (outputFile);
		}
	}
	return get_exitstatus();
}

