#include <iostream>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <wait.h>

using namespace std;

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

void chomp(char *string, char delim);
void cpplines(FILE *pipe, char *filename);

int main(int argc, char **argv)
{
	int arg;
	int arg_error = 0;
	int oc_error = 0;
	string input_file = "";
	char *filename;
	while((arg =  getopt(argc, argv, "ly@:D:")) != -1)
		{
		switch (arg)
			{
				case 'l':
				   printf("%s\n", "-l");
				   break;
				 case 'y':
					 printf("%s\n", "-y");
					break;
				 case '@':
					 printf("flag: %s\n",optarg);
					break;
				 case 'D':
					 printf("file: %s\n",optarg);
					break;
				case ':':
					cerr << optopt << " Requires Input" << endl;
					arg_error = 1;
					break;
				case '?':
					cerr << optopt << " is not a valid argument" << endl;
					arg_error = 1;
					break;
			}
		}
		if(arg_error != 0){
			cout << "error" << endl;

		}else{
			cout << "input file: " << argv[optind] << endl;
				input_file = argv[optind];

				if(input_file.compare(input_file.length()-2, input_file.length(), "oc") != 0 ){
					cout <<"unknonw file extention" <<endl;
					exit(1);
				filename = argv[optind];
			}
		}

		if(oc_error == 0){
			filename = "foo.oc";
			printf("%s", filename);
			string cmd = CPP + " " + filename;
			printf("command=\"%s\"\n", cmd.c_str());
			FILE *pipe = popen(cmd.c_str(), "r");
			if(pipe == NULL){
				printf("error: %s\n", cmd.c_str());
			}else{
				cpplines(pipe, filename);
				int pclose_rc = pclose(pipe);
				printf("popen status: %d\n",pclose_rc);
			}

		}else{
			exit(1);
		}
}

void cpplines (FILE *pipe, char *filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char *fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
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
      }
      ++linenr;
   }
}

void chomp (char *string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char *nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}
