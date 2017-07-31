//*****************************************************
//*  Nicholas Pappas
//*  Riley Honbo
//*  nhpappas@ucsc.edu
//*  rhonbo@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 2
//*****************************************************

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
#include "lyutils.h"
#include "astree.h"

const string CPP = "/usr/bin/cpp -nostdinc";
FILE* tok_output;
constexpr size_t LINESIZE = 1024;
string cpp_command;
//Included cpp_popen/pclose & scan_opts
void scan (string name) {
    int pos = name.find('.');       //get rid of current suffix ".oc"
    name.erase(pos, name.length());
    name += ".tok";                 //C/C++ casting semantics.
    //use fopen to create a .tok file
    tok_output = fopen(name.c_str(),"w"); 
    
    if (tok_output == NULL){
    fprintf (stderr, "Error: '.tok' file failed to open");
    }else{
        for(;;){
            int tok = yylex();
            if (tok == YYEOF) break;
            fprintf(tok_output,"%lu %lu.%03lu %3d %-10.10s (%s)\n",\
            lexer::lloc.filenr, lexer::lloc.linenr,\
            lexer::lloc.offset, tok, parser::get_tname(tok), yytext);
            string_set::intern(yytext);
            DEBUGF('m', "token=%d", yytext);
        }
    }
    fclose (tok_output);
}

void cpp_popen (const char* filename) {
   cpp_command = CPP + " " + filename;
    yyin = popen (cpp_command.c_str(), "r");
    if (yyin == NULL) {
        syserrprintf (cpp_command.c_str());
        exit (exec::exit_status);
    }else {
        if (yy_flex_debug) {
            fprintf (stderr, "-- popen (%s), fileno(yyin) = %d\n",\
            cpp_command.c_str(), fileno (yyin));
        }
        lexer::newfilename (cpp_command);
        scan(filename);
   }
}

void cpp_pclose() {
   int pclose_rc = pclose (yyin);
   eprint_status (cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) exec::exit_status = EXIT_FAILURE;
}

void scan_opts (int argc, char** argv) {
   int option;
   yy_flex_debug = false;
   yydebug = false;
   string filename = argv[argc-1];
   int pos = filename.find('.'); //Determine where suffix begins: ".oc"
   //Compare suffix to ".oc"
   if(filename.substr(pos, filename.length()) != ".oc"){ 
       fprintf (stderr, "Error: Given file is not '.oc'. \
       Please run on a '.oc' file.");
   }else{ //The file is ".oc" so we can check all the flag options 
      while((option = getopt(argc, argv, "@:D:ly")) != -1){
         switch(option){
            case 'l': 
            yy_flex_debug = true;
               break;
            case 'y': 
            yydebug = true;
               break;
            case '@':
               //Note optarg is a char* in getopt()
               set_debugflags(optarg); 
               break;
            case 'D':
               cpp_command = CPP+" -D"+optarg+" "+filename;
               break;
            }
        }
    }
    cpp_popen(filename.c_str());
}
//Our code begins here

// Run cpp against the lines of the file.

int main (int argc, char** argv) {
   int exit_status = EXIT_SUCCESS;
   if(argc <= 1){
       printf ("Error: Too few arguments. \
       Please run 'oc ('-@flags|D|l|y') file.oc'\n");
       exit(EXIT_FAILURE);
   }
   //Get file path from ARGV //The last argument in ARGV is program.oc
   exec::execname = basename (argv[0]);
   string ocname = argv[argc-1];
   //command = CPP + " " + filename;
   scan_opts(argc, argv);
   cpp_pclose();
   //Write string_set to file & dump
   int pos = ocname.find('.'); //get rid of current suffix ".oc"
   ocname.erase(pos, ocname.length()); 
   ocname += ".str"; //C/C++ casting semantics.
   //use fopen to create a .str file
   FILE* str_output = fopen(ocname.c_str(),"w"); 
   string_set::dump(str_output);
   fclose(str_output);
   return exit_status;
}

