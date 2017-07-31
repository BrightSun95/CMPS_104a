//*****************************************************
//*  Nicholas Pappas
//*  Riley Honbo
//*  nhpappas@ucsc.edu
//*  rhonbo@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 4
//*****************************************************

#include <bitset>
#include <iostream>
#include <string>
#include <unordered_map>

#include "symtable.h"
#include "astree.h"
#include "lyutils.h"
#include "string_set.h"


vector<symbol_table*> symStack;
vector<int> blockCounter;
int nextBlock = 1;
FILE* sym_output;

symbol* newsymbol(astree* node){
   if(symStack.empty() )
   {
      symStack.push_back(new symbol_table);
      symStack.push_back(nullptr);
   }
   if(blockCounter.empty() )
   {
      blockCounter.push_back(0);
   }
   symbol* a = new symbol();
   a->attributes = node->attributes;
   a->fields = nullptr;
   a->fieldName = "";
   a->typeID = "";
   a->blockNum = blockCounter.back();
   a->filenr = node->lloc.filenr;
   a->linenr = node->lloc.linenr;
   a->offset = node->lloc.offset;
   a->parameters = nullptr;
   return a;
}


const char* check_attr(astree* node, symbol* sym){
   string out = "";
   attr_bitset attr = node->attributes;
    if (attr[ATTR_void] == 1) {
        out += "void ";}
    if (attr[ATTR_field]) {
    out += "field {" + sym->fieldName + "} ";}
    if (attr[ATTR_array] == 1) {
        out += "array ";}
    if (attr[ATTR_int] == 1) {
        out += "int ";}
    if (attr[ATTR_null] == 1) {
        out += "null ";}
    if (attr[ATTR_string] == 1) {
        out += "string ";}
    if (attr[ATTR_struct] == 1) {
        out += "struct ";}
    if (attr[ATTR_typeid] == 1) {
        //need to change so it displays "\"typeid\""
        out += "\"" + sym->typeID + "\" ";}
        //out += "typeid ";}
    if (attr[ATTR_function] == 1) {
        out += "function ";}
    if (attr[ATTR_variable] == 1) {
        out += "variable ";}
    if (attr[ATTR_lval] == 1) {
        out += "lval ";}
    if (attr[ATTR_param] == 1) {
        out += "param ";}
    if (attr[ATTR_const] == 1) {
        out += "const ";}
    if (attr[ATTR_vreg] == 1) {
        out += "vreg ";}
    if (attr[ATTR_vaddr] == 1) {
        out += "vaddr ";}
    if (attr[ATTR_bool] == 1) {
        out += "bool ";}
    if (attr[ATTR_char] == 1) {
        out += "char ";}
    
   return out.c_str();
}

void check_array(astree* node){
   
   switch(node->children[0]->symbol){
      
      case TOK_CHAR:       node->attributes[ATTR_char] = 1;
                           node->attributes[ATTR_lval] = 1;
                           break;

      case TOK_INT:        node->attributes[ATTR_int] = 1;  
                           node->children[0]-> 
                              attributes[ATTR_variable] = 1;
                           node->children[0]-> 
                              attributes[ATTR_int] = 1;
                           node->
                              attributes[ATTR_lval] = 1;
                           break;
      case TOK_BOOL:       node->attributes[ATTR_bool]= 1;
                           node->attributes[ATTR_const] = 1;
                           node->attributes[ATTR_lval] = 1;
                           break;
     } 
}

void set_attribute(astree* node){
   switch(node->symbol)
   {
      case TOK_VOID:       node->attributes[ATTR_void] = 1;
                           if (node->children.size() > 0)
                           node->children[0]-> 
                              attributes[ATTR_void] = 1;
                           break;
      case TOK_BOOL:       node->attributes[ATTR_bool]= 1;
                           node->attributes[ATTR_const] = 1;
                           node->attributes[ATTR_lval] = 1;
                           if (node->children.size() > 0)
                           node->children[0]-> 
                                attributes[ATTR_bool] = 1;
                           break;
      case TOK_CHAR:       node->
                                 attributes[ATTR_char] = 1;
                           node->
                                 attributes[ATTR_lval] = 1;
                           if (node->children.size() > 0)
                           node->children[0]-> 
                                 attributes[ATTR_char] = 1;
                           break;
      case TOK_INT:        node->attributes[ATTR_int] = 1; 
                           node->attributes[ATTR_lval] = 1; 
                           if (node->children.size() > 0)
                           node->children[0]-> 
                                 attributes[ATTR_int] = 1;
                           break;
      case TOK_NULL:       node->attributes[ATTR_null] = 1;
                           node->attributes[ATTR_const] = 1;
                           break;
      case TOK_STRING:     node->attributes[ATTR_string] = 1;
                           node->attributes[ATTR_lval] = 1;
                           if (node->children.size() > 0)
                           node->children[0]-> 
                                attributes[ATTR_string] = 1;
                           break;
      case TOK_STRUCT:     node->attributes[ATTR_struct] = 1;
                           node->attributes[ATTR_typeid] = 1;
                           node->children[0]-> 
                              attributes[ATTR_struct] = 1;
                           node->attributes[ATTR_variable] = 0;
                           node->attributes[ATTR_lval] = 0;
                           node->children[0]->
                              attributes[ATTR_variable] = 0;
                           node->children[0]->
                              attributes[ATTR_lval] = 0;
                           break;
      case TOK_ARRAY:      node->attributes[ATTR_array] = 1;
                           //check_array(node);
                           break;
      
      case TOK_FUNCTION:   node->attributes[ATTR_function] = 1;
                        node->children[0]->children[0]->
                           attributes[ATTR_function] = 1;
                           node->attributes[ATTR_variable] = 0;
                           node->attributes[ATTR_lval] = 0;
                        node->children[0]->children[0]->
                           attributes[ATTR_variable] = 0;
                        node->children[0]->children[0]->
                           attributes[ATTR_lval] = 0;
                           break;
      case TOK_PROTOTYPE:
                        node->children[0]->children[0]->
                           attributes[ATTR_variable] = 0;
                        node->children[0]->children[0]->
                           attributes[ATTR_lval] = 0;
                           break;
      case TOK_PARAMLIST:  for (size_t index = 0;
                                 index < node->children.size(); ++index)
                        node->children[index]->children[0]->
                                 attributes[ATTR_param] = 1;
                           break;
      case TOK_STRINGCON:  node->attributes[ATTR_string] = 1;
                           node->attributes[ATTR_const] = 1;
                           node->attributes[ATTR_lval] = 1;
                           break;
      case TOK_CHARCON:    node->attributes[ATTR_char] = 1;
                           node->attributes[ATTR_const] = 1;  
                           node->attributes[ATTR_lval] = 1;
                           break;
      case TOK_INTCON:     node->attributes[ATTR_int] = 1;
                           node->attributes[ATTR_const] = 1;
                           node->attributes[ATTR_lval] = 1;
                           break;
      case TOK_FIELD:      node->attributes[ATTR_field] = 1;
                           if("TOK_TYPEID" == 
                           string(parser::get_tname(node->symbol))){
                              node->attributes[ATTR_struct] = 1;}
                           break;
      case TOK_TYPEID:     node->attributes[ATTR_typeid] = 1;
                           for(size_t index = 0; 
                              index < node->children.size(); ++index)
                           {
                      node->children[index]->
                               attributes[ATTR_typeid] = 1;
                      node->children[index]->
                              attributes[ATTR_struct] = 1;
                      break;
                      }
      case TOK_IDENT:      node->attributes[ATTR_lval] = 1;
                           node->attributes[ATTR_variable] = 1;
                           break;
      case TOK_VARDECL:    node->attributes[ATTR_variable] = 1;
                           break;
      case TOK_DECLID:     node->attributes[ATTR_lval] = 1;
                           node->attributes[ATTR_variable] = 1;
                           break;
      default:           
                           break;
   }
}

void insert_symbol (symbol_table table, const string* key, 
                     symbol* sym, astree* node) {
   table[key] = sym;
   for (size_t size = 1; size < blockCounter.size(); ++size) {
      fprintf(sym_output, "   ");
   }
   fprintf (sym_output, "%s (%zu.%zu.%zu) {%zu} %s\n", key->c_str(), 
            sym->filenr, sym->linenr, sym->offset, sym->blockNum, 
            check_attr (node, sym));
}

void make_struct_symbol (astree* root) {
   symbol* sym = newsymbol(root->children[0]);
   const string* key;
   symbol_table fields;
   sym->typeID = root->children[0]->lexinfo->c_str();
   sym->fields = &fields;
   key = root->children[0]->lexinfo;
   insert_symbol(*symStack[0], key, sym, root->children[0]);

   if(root->children.size() > 1){
      astree* fieldlist = root->children[1];
      for (size_t index = 0;\
     index < fieldlist->children.size(); ++index){
         astree* field = fieldlist->children[index]->children[0];
         sym = newsymbol(field);
         key = field->lexinfo;
         sym->fieldName = root->children[0]->lexinfo->c_str();
         if("TOK_TYPEID" == 
       string(parser::get_tname(fieldlist->children[index]->symbol))){
            sym->typeID = fieldlist->children[index]->lexinfo->c_str();
            }
         fprintf (sym_output, "   ");
         insert_symbol(fields, key, sym, field);
      }
   }
}

void make_block (astree* root) {
      //increment nextBlock
      blockCounter.push_back(nextBlock);
      nextBlock++;
      //push new symbol_table on stack
      symStack[blockCounter.back()] = new symbol_table;
      symStack.push_back(nullptr);
      traversing(root);
      blockCounter.pop_back();
}

void make_func_symbol(astree* root){
   astree* function = root->children[0]->children[0];
   symbol* sym = newsymbol(function);
   const string* key;
   vector<symbol*> params;
   sym->typeID = root->children[0]->lexinfo->c_str();
   sym->parameters = &params;
   key = function->lexinfo;
   insert_symbol (*symStack[0], key, sym, function);
   
   astree* paramlist = root->children[1];
   for (size_t index = 0; index < paramlist->children.size(); ++index){
      astree* param = paramlist->children[index]->children[0];
      sym = newsymbol(param);
      key = param->lexinfo;
      if("TOK_TYPEID" == 
        string(parser::get_tname(paramlist->children[index]->symbol))){
         sym->typeID = paramlist->children[index]->lexinfo->c_str();
         }
      ++sym->blockNum;
      params.push_back(sym);
      fprintf(sym_output, "   ");
      insert_symbol (*symStack[0], key, sym, param);
   }
   fprintf(sym_output, "\n");
   make_block(root->children[2]); 
}

void make_call_symbol(astree* root){
   astree* call = root->children[0];
   symbol* sym = newsymbol(call);
   const string* key;
   vector<symbol*> params;
   sym->typeID = root->children[0]->lexinfo->c_str();
   sym->parameters = &params;
   key = call->lexinfo;
   insert_symbol (*symStack[0], key, sym, call);
   
   //astree* paramlist = root->children[1];
   for (size_t index = 1; index < root->children.size(); ++index){
      astree* call = root->children[index];
      sym = newsymbol(call);
      key = call->lexinfo;
      ++sym->blockNum;
      params.push_back(sym);
      fprintf(sym_output, "   ");
      insert_symbol (*symStack[0], key, sym, call);
   }
   fprintf(sym_output, "\n");
   //make_block(root->children[2]); 
}

void make_proto_symbol(astree* root){
   astree* proto;
   if("TOK_ARRAY" == 
     string(parser::get_tname(root->children[0]->symbol))){
      proto = root->children[0]->children[1];
      proto->attributes = root->children[0]->children[0]->attributes;
      proto->attributes[ATTR_array] = 1;
      }
   else{proto = root->children[0]->children[0];}
   symbol* sym = newsymbol(proto);
   const string* key;
   vector<symbol*> params;

   sym->parameters = &params;
   key = proto->lexinfo;
   insert_symbol (*symStack[0], key, sym, proto);
   
   /*astree* paramlist = root->children[1];
   for (size_t index = 0; 
         index < paramlist->children.size(); ++index){
      astree* param = paramlist->
         children[index]->children[0];
      sym = newsymbol(param);
      ++sym->blockNum;
      params.push_back(sym);
   }*/
   astree* paramlist = root->children[1];
   for (size_t index = 0; index < paramlist->children.size(); ++index){
      astree* proto = paramlist->children[index]->children[0];
      sym = newsymbol(proto);
      key = proto->lexinfo;
      if("TOK_TYPEID" == 
        string(parser::get_tname(paramlist->children[index]->symbol))){
         sym->typeID = paramlist->children[index]->lexinfo->c_str();
         }
      ++sym->blockNum;
      params.push_back(sym);
      fprintf(sym_output, "   ");
      insert_symbol (*symStack[0], key, sym, proto);
   }
   fprintf(sym_output, "\n");
   //make_block(root->children[2]); 
}

void make_vardecl(astree* root){
   astree* vardecl = root->children[0]->children[0];
   symbol* sym = newsymbol(vardecl);
   const string* key;
   key = vardecl->lexinfo;
  if("TOK_TYPEID" == 
     string(parser::get_tname(root->children[0]->symbol))){
       sym->typeID = root->children[0]->lexinfo->c_str();
   }
   /*printf(parser::get_tname(root->children[0]->symbol));
   printf("\n");
   printf(root->children[0]->lexinfo->c_str());
   printf("\n");*/
   insert_symbol (*symStack[blockCounter.back()], 
                  key, sym, vardecl);
}

void run_attr(astree* root){
   for( size_t index = 0; index < root->children.size(); ++index){
      run_attr(root->children[index]);
   }
   set_attribute(root);
}

void traversing(astree* root){
   for (size_t index = 0; index < root->children.size(); ++index) {
      
      int nodesymbol = root->children[index]->symbol;
      switch(nodesymbol){
         case TOK_STRUCT:  
            make_struct_symbol(root->children[index]);
            fprintf(sym_output, "\n");
            break;
         case TOK_FUNCTION: 
              make_func_symbol(root->children[index]); 
            fprintf(sym_output, "\n");
            break;
         case TOK_PROTOTYPE:
            make_proto_symbol(root->children[index]); 
            fprintf(sym_output, "\n");
            break;
         case TOK_CALL:
            make_call_symbol(root->children[index]); 
            fprintf(sym_output, "\n");
            break;
         case TOK_VARDECL:
            make_vardecl(root->children[index]);
            break;
         case TOK_IF:
            make_block(root->children[index]->children[1]);
            break;
         case TOK_IFELSE:
            make_block(root->children[index]->children[1]);
            make_block(root->children[index]->children[2]);
            break;
         default:
            break;
      }
   }  
}
