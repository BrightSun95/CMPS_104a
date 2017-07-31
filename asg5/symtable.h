//*****************************************************
//*  Nicholas Pappas
//*  Riley Honbo
//*  nhpappas@ucsc.edu
//*  rhonbo@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 5
//*****************************************************

#ifndef __SYMBOLTABLE_H__
#define __SYMBOLTABLE_H__


#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "auxlib.h"
#include "astree.h"

extern FILE* sym_output;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
       ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
struct astree;
//struct location;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = pair<string*,symbol*>;

struct symbol {
   attr_bitset attributes;
   symbol_table* fields;
   string typeID;
   string fieldName;
   size_t filenr;
   size_t linenr;
   size_t offset;
   size_t blockNum;
   vector<symbol*>* parameters;
};

const char* check_attr(astree* node, symbol* sym);
void set_attribute(astree* node);
void make_struct_symbol(astree* root);
void make_block(astree* root);
void make_func_symbol(astree* root);
void make_proto_symbol(astree* root);
void run_attr(astree* root);
void traversing(astree* root);


#endif
