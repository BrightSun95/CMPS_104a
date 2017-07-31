//*****************************************************
//*  Nicholas Pappas
//*  Riley Honbo
//*  nhpappas@ucsc.edu
//*  rhonbo@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 4
//*****************************************************

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"
#include "symtable.h"
using attr_bitset = bitset<17>;

struct location {
   size_t filenr;
   size_t linenr;
   size_t offset;
};

struct astree {

   // Fields.
   int symbol;               // token code
   location lloc;            // source location
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   attr_bitset attributes;
   int blocknr;

   // Functions.
   void really_prints_ast();
   astree (int symbol, const location&, const char* lexinfo);
   ~astree();
   astree* adopt (astree* child1, astree* child2 = nullptr,\
                  astree* child3 = nullptr);
   astree* adopt_sym (astree* child, int symbol);
   astree* swap_sym (astree* root, int symbol);
   void dump_node (FILE*);
   void dump_tree (FILE*, int depth = 0);
   static void dump (FILE* outfile, astree* tree);
   static void print (FILE* outfile, astree* tree, int depth = 0);
   static void debug_print (astree* tree, int depth);
};

void destroy (astree* tree1, astree* tree2 = nullptr);

void errllocprintf (const location&, const char* format, const char*);

#endif

