//*********************************************
//*  Nicholas Pappas
//*  nhpappas@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 1
//*********************************************
//---------------------------------------------
// main.cpp
// Reads in options and arguments using getopt(3)
// Gets basename of file and creates basename.str
// Reads input file, tokenizes it, inputs toks 
// into the string_set ADT 
// (a Hash Table), and dumps results to basename.str
//----------------------------------------------

#include <string>
#include <unordered_set>

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
using namespace std;
#include "string_set.h"
#include "auxlib.h"

const string CPP = "/usr/bin/cpp -nostdinc";
constexpr size_t LINESIZE = 1024;

// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

// Run cpp against the lines of the file.
void cpplines (FILE* pipe, const char* filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      // printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
      int sscanf_rc = sscanf (buffer, "# %d \"%[^\"]\"",
                              &linenr, inputname);
      if (sscanf_rc == 2) {
         // printf ("DIRECTIVE: line %d file 
         //\"%s\"\n", linenr, inputname);
         continue;
      }
      char* savepos = NULL;
      char* bufptr = buffer;
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         // printf ("token %d.%d: [%s]\n",
         //        linenr, tokenct, token); 
        string_set::intern(token);
      }
      ++linenr;
   }
}

int main (int argc, char** argv) {
   int exit_status = EXIT_SUCCESS;
   if(argc <= 1){
       printf ("Error: Too few arguments. \
       Please run 'oc ('-@flags|D|l|y') file.oc'\n");
       exit(EXIT_FAILURE);
   }
   
   const char* execname = basename(argv[0]); //Get file path from ARGV
   //The last argument in ARGV is program.oc
   string filename = argv[argc-1]; 
   string ocname = filename;
   string command = CPP + " " + filename;
   int pos = filename.find('.'); //Determine where suffix begins: ".oc"
   int option;
   
   //Compare suffix to ".oc"
   if(filename.substr(pos, filename.length()) != ".oc"){ 
       fprintf (stderr, "Error: Given file is not '.oc'. \
       Please run on a '.oc' file.");
   }else{ //The file is ".oc" so we can check all the flag options 
      while((option = getopt(argc, argv, "@:D:ly")) != -1){
         switch(option){
             //yy_flex_debug = 1; //Debug yylex() Not used in P1
            case 'l': 
               break;
            case 'y': //yydebug = 1; //Debug yyparse() Not used in P1
               break;
            case '@':
               //Note optarg is a char* in getopt()
               set_debugflags(optarg); 
               break;
            case 'D':
               command = CPP+" -D"+optarg+" "+filename;
               break;
            }
        }
    }
      
   //Tokenize the input file after it runs through CPP 
   //and puts toks in string_set
   //.c_str() for using string as char* args
   FILE* pipe = popen (command.c_str(), "r"); 
   if(pipe == NULL){
      exit_status = EXIT_FAILURE;
      fprintf (stderr, "%s: %s: %s\n",execname, \
      command.c_str(), strerror (errno));
   }else{
       cpplines (pipe, filename.c_str());
       int pclose_rc = pclose (pipe);
       eprint_status (command.c_str(), pclose_rc);
       if (pclose_rc != 0) exit_status = EXIT_FAILURE;
   }
   
   //Write string_set to file & dump
   pos = ocname.find('.');
   //get rid of current suffix ".oc"
   ocname.erase(pos, ocname.length()); 
   ocname += ".str"; //C/C++ casting semantics.
   //use fopen to create a .str file
   FILE* output = fopen(ocname.c_str(),"w"); 
   
   string_set::dump(output);
   fclose(output);
   return exit_status;
}
