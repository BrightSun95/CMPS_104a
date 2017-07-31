//*****************************************************
//*  Nicholas Pappas
//*  Riley Honbo
//*  nhpappas@ucsc.edu
//*  rhonbo@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 4
//*****************************************************

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"
#include "symtable.h"

FILE* ast_output;

void astree::really_prints_ast(){
   ast_output = fopen(anothername.c_str(),"w"); 
   astree::print(ast_output, parser::root);
}

astree::astree (int symbol_, const location& lloc_, const char* info) {
   symbol = symbol_;
   lloc = lloc_;
   lexinfo = string_set::intern (info);
   attributes = 0;
   blocknr = 0;
   // vector defaults to empty -- no children
}

astree::~astree() {
   while (not children.empty()) {
      astree* child = children.back();
      children.pop_back();
      delete child;
   }
   if (yydebug) {
      fprintf (stderr, "Deleting astree (");
      astree::dump (stderr, this);
      fprintf (stderr, ")\n");
   }
}

astree* astree::adopt (astree* child1, astree* child2, astree* child3) {
   //cout << "In adopt: ";
   if (child1 != nullptr){
        children.push_back (child1);
        //cout << "Child 1 " << parser::get_tname(child1->symbol);
   }
   if (child2 != nullptr){
        children.push_back (child2);
        //cout << " Child 2 " << parser::get_tname(child2->symbol);
   }
   if (child3 != nullptr){
        children.push_back (child3);
        //cout << " Child 3 " << parser::get_tname(child3->symbol);
   }
   //cout << endl;
   //astree::debug_print(parser::root,0);
   //astree::really_prints_ast();
   return this;
}

astree* astree::adopt_sym (astree* child, int symbol_) {
   //cout << "AdoptSym" << endl;
   symbol = symbol_;
   return adopt (child);
}

astree* astree::swap_sym (astree* root, int symbol) {
    root->symbol = symbol;
    return root;
}

void astree::dump_node (FILE* outfile) {
   fprintf (outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
            this, parser::get_tname (symbol),
            lloc.filenr, lloc.linenr, lloc.offset,
            lexinfo->c_str());
   for (size_t child = 0; child < children.size(); ++child) {
      fprintf (outfile, " %p", children.at(child));
   }
}

void astree::dump_tree (FILE* outfile, int depth) {
   fprintf (outfile, "%*s", depth * 3, "");
   dump_node (outfile);
   fprintf (outfile, "\n");
   for (astree* child: children) child->dump_tree (outfile, depth + 1);
   fflush (NULL);
}

void astree::dump (FILE* outfile, astree* tree) {
   if (tree == nullptr) fprintf (outfile, "nullptr");
                   else tree->dump_node (outfile);
}

void astree::print (FILE* outfile, astree* tree, int depth) {
   const char *tname = parser::get_tname(tree->symbol);
   if (strstr (tname, "TOK_") == tname) tname += 4;
   for (int i = 0; i < depth; i++) {
      fprintf(outfile, "|   ");
   }
   //fprintf (outfile, "| %*s", depth * 3, "|");
   fprintf (outfile, "%s \"%s\" (%zd.%zd.%zd)\n",
            tname, tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset);
   for (astree* child: tree->children) {
      astree::print (outfile, child, depth + 1);
   }
   //cout << "Printing: " << tname << endl;
}

void astree::debug_print (astree* tree, int depth) {
   printf ("; %*s", depth * 3, "");
   printf ("%s \"%s\" (%zd.%zd.%zd)\n",
            parser::get_tname (tree->symbol), tree->lexinfo->c_str(),
            tree->lloc.filenr, tree->lloc.linenr, tree->lloc.offset);
   for (astree* child: tree->children) {
      astree::debug_print(child, depth + 1);
   }
}

void destroy (astree* tree1, astree* tree2) {
   //cout << "Tree is being destroyed: ";
   if (tree1 != nullptr){ 
   //cout << "tree 1 " << parser::get_tname(tree1->symbol);
   delete tree1;}
   if (tree2 != nullptr){ 
   //cout << "tree 2 " << parser::get_tname(tree2->symbol);
   delete tree2;}

   //cout << endl;
}

void errllocprintf (const location& lloc, const char* format,
                    const char* arg) {
   static char buffer[0x1000];
   assert (sizeof buffer > strlen (format) + strlen (arg));
   snprintf (buffer, sizeof buffer, format, arg);
   errprintf ("%s:%zd.%zd: %s", 
              lexer::filename (lloc.filenr), lloc.linenr, lloc.offset,
              buffer);
}
