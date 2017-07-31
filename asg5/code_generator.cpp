//*****************************************************
//*  Nicholas Pappas
//*  Riley Honbo
//*  nhpappas@ucsc.edu
//*  rhonbo@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 5
//*****************************************************

#include <iostream>
#include <string>
#include <vector>

#include "symtable.h"
#include "code_generator.h"

size_t reg_counter = 1;

FILE* oil_output;

const char* give_name(astree* node, string current_name){
   string new_name;
   int symbol = node->symbol;
   if(node->blocknr != 0){
      switch (symbol){
         case TOK_WHILE:
         case TOK_IF:
         case TOK_IFELSE:
                        new_name = current_name + "_"
                           + to_string(node->lloc.filenr) + "_"
                           + to_string(node->lloc.linenr) + "_"
                           + to_string(node->lloc.offset) + ":;";
                        break;
         case NUMBER:   new_name = current_name;
                        break;
         case TOK_FUNCTION:  new_name = "__" + current_name;
                        break;
         default:         new_name = "_" +
                                to_string(node->blocknr)
                                 + "_" + *node->lexinfo;
      }
   }else{
      switch (symbol){
         case TOK_STRUCT:   new_name = "s_" + current_name;
                        break;
         case TOK_TYPEID:   new_name = "f_" + *node->lexinfo
                                 + "_" + current_name;
                        break;
         case TOK_WHILE:
         case TOK_IF:
         case TOK_IFELSE:
                        new_name = current_name + "_"
                           + to_string(node->lloc.filenr) + "_"
                           + to_string(node->lloc.linenr) + "_"
                           + to_string(node->lloc.offset) + ":;";
                        break;
         case TOK_DECLID:   new_name = "_"
                           + to_string(node->blocknr)
                           + "_" + *node->lexinfo;
                        break;
         default:         new_name = "__" + current_name;
                        break;
      }
   }
   return new_name.c_str();
}

const char* convert_type(astree* node, const string* struct_name){
   string current_type = *node->lexinfo;
   string new_type;
   if(current_type == "int"){
      new_type = "int";
   }else if(current_type == "char"){
      new_type = "char";
   }else if(current_type == "bool"){
      new_type = "char";
   }else if(current_type == "string"){
      new_type = "char*";
   }else{
      if(struct_name == nullptr)
         current_type = "ERR" + current_type;
         return current_type.c_str();
      astree* a_struct = node;
      a_struct->symbol = TOK_STRUCT;
      a_struct->lexinfo = struct_name;
      new_type = "struct " + 
         string(give_name(a_struct, current_type)) + "*";
   }
   return new_type.c_str();
}

void write_struct(astree* child, int depth){
   astree* struct_name = child->children[0];
   string _depth = string(depth * 3, ' ');
   fprintf (oil_output, "struct %s{\n", 
            give_name(child, *struct_name->lexinfo));
   depth++;
   for(size_t i = 1; i < child->children.size(); ++i){
      astree* type  = child->children[i];
      astree* field = type->children[0];
      string current_name = *field->lexinfo;
      field->lexinfo = struct_name->lexinfo;
      fprintf (oil_output, "%s%s %s;\n", _depth.c_str(),
            convert_type(type, struct_name->lexinfo),
            give_name(child, *struct_name->lexinfo));
   }
   fprintf (oil_output, "};\n");
}

void write_strconst(astree* node, int depth){
   if(node->symbol == '='){
      if(node->children[1]->symbol == TOK_STRINGCON){
         string plzwork = *node->children[1]->lexinfo;
         fprintf (oil_output, "char* %s = %s\n", give_name(node,
            *node->children[0]->children[0]->lexinfo),
            plzwork.c_str());
      }
   }else{
      for (astree* child: node->children){
         write_strconst(child, depth);
      }
   }
}

void write_return(astree* node, int depth){
   astree* return_val = node->children[0];
   string _depth = string(depth * 3, ' ');
   fprintf (oil_output, "%s", _depth.c_str());
   if(return_val != nullptr){
      if(return_val->symbol == TOK_IDENT){
         fprintf (oil_output, "return %s;\n", 
            give_name(return_val, *return_val->lexinfo));
      }else{
         string plzwork = *return_val->lexinfo;
         fprintf (oil_output, "return %s;\n", plzwork.c_str());
      }
   }else{
      fprintf (oil_output, "return;\n");
   }
}

void write_binary(astree* node){
   astree* left = node->children[0];
   astree* right = node->children[1];
   string plzwork = *node->lexinfo;
   fprintf (oil_output, "char b%zd = %s %s %s;\n", reg_counter++,
      give_name(left, *left->lexinfo), plzwork.c_str(),
      give_name(right, *right->lexinfo));
}

void write_unary(astree* node){
   string plzwork = *node->lexinfo;
   fprintf (oil_output, "char b%zd = %s%s;\n", reg_counter++,
      plzwork.c_str(), give_name(node, *node->lexinfo));
}

void write_conditional(astree* node){
   size_t size = node->children.size();
   if(size == 2){
      write_binary(node);
   }else if(size==1){
      write_unary(node);
   }else{
      fprintf (oil_output, "char b%zd = %s;\n", reg_counter++,
         give_name(node, *node->lexinfo));
   }
}

void write_while(astree* node, int depth){
   fprintf (oil_output, "%s\n", give_name(node, *node->lexinfo));
   write_conditional(node->children[0]);
   fprintf (oil_output, "if(!b%zd) goto break_%zd_%zd_%zd;\n", 
      reg_counter-1, node->lloc.filenr, node->lloc.linenr,
      node->lloc.offset);
   oil_stuff(node->children[1], depth, node);
   fprintf (oil_output, "goto %s\n", 
      give_name(node, *node->lexinfo));
   fprintf (oil_output, "break_%zd_%zd_%zd:\n", 
      node->lloc.filenr, node->lloc.linenr, node->lloc.offset);

}

void write_call(astree* node, int depth){
   node->children[0]->symbol = TOK_FUNCTION;
   string _depth = string(depth * 3, ' ');
   fprintf (oil_output, "%s%s (", _depth.c_str(),
      give_name(node->children[0], *node->children[0]->lexinfo));
   for (size_t i = 1; i < node->children.size(); i++){
      astree* arg = node->children[i];
      fprintf (oil_output, "%s", give_name(arg, *arg->lexinfo));
      if(i+1 != node->children.size()){
         fprintf (oil_output, ", ");
      }
   }
   fprintf (oil_output, ")");
}

void write_expression(astree* node, int depth){
   astree* first = node->children[0];
   string plzwork, plzwork2;
   switch (first->symbol){
      case TOK_IDENT: 
         plzwork = *node->lexinfo;
         fprintf (oil_output, "%s %s ", 
            give_name(first,*first->lexinfo), plzwork.c_str());
                  break;
      case '+':
      case '-':      
         if(first->children.size() == 1){
            plzwork = *first->lexinfo;
            plzwork2 = *first->children[0]->lexinfo;
            fprintf (oil_output, "%s%s", 
               plzwork.c_str(), plzwork2.c_str());
                  break;
                  }
      case '/':
      case '*':      
         write_expression(first, depth);
         plzwork = *first->lexinfo;
            fprintf (oil_output, "%s ", plzwork.c_str());
                  break;
      case TOK_STRINGCON:
      case TOK_CHARCON:
      case NUMBER:
         plzwork = *first->lexinfo;
         plzwork2 = *node->lexinfo;
         fprintf (oil_output, "%s %s ", 
               plzwork.c_str(), plzwork2.c_str());
                  break;
      case TOK_CALL: 
         write_call(first, depth);
                  break;
   }
   astree* second = node->children[1];
   switch (second->symbol){
      case TOK_IDENT: 
         fprintf (oil_output, "%s", 
            give_name(second,*second->lexinfo));
                  break;
      case TOK_STRINGCON:
      case TOK_CHARCON:
      case NUMBER:
      case TOK_CALL: write_call(second, depth);
         fprintf (oil_output, " ");
                  break;
   }
}

string make_type_reg(string s){
   string stuff;
   if(s == "char"){
      stuff = "c";
   }else if(s == "int"){
      stuff = "i";
   }else if(s == "string"){
      stuff = "s";
   }else{
      stuff = "i";
   }
   return stuff;
}

void write_new(astree* node, int depth){
   astree* new_type = node->children[0];
   string plzwork;
   if(new_type->symbol == TOK_TYPEID){
      plzwork = *new_type->lexinfo;
      fprintf (oil_output, 
         "struct %s* p%zd = xcalloc (1, sizeof (struct %s));\n",
         plzwork.c_str(), reg_counter++, 
         give_name(new_type, *new_type->lexinfo));
   }else if(node->symbol == TOK_NEWARRAY){
      plzwork = *node->children[1]->lexinfo;
      fprintf (oil_output, 
         "%s* p%zd = xcalloc (%s, sizeof (%s));\n",
         convert_type(node->children[0],node->children[0]->lexinfo),
         reg_counter++, plzwork.c_str(),
         give_name(node->children[0],*node->children[0]->lexinfo));
   }else if(node->symbol == TOK_NEWSTRING){
      plzwork = node->children[0]->lexinfo->length()-2;
      fprintf (oil_output, 
         "char* p%zd = xcalloc (%s, sizeof (char));\n",
         reg_counter++, plzwork.c_str());
   }else{
      plzwork = *node->lexinfo;
      fprintf (oil_output,"ERROR: %s;\n", plzwork.c_str());
   }
   depth = depth + 1;
}

void write_eq(astree* node, int depth){
   astree* left = node->children[0];
   astree* right = node->children[1];
   string _depth = string(depth * 3, ' ');
   string plzwork;
   if(left->symbol != TOK_IDENT){
      switch (right->symbol){
         case TOK_IDENT: 
         fprintf (oil_output, "%s%s %s = %s;\n", _depth.c_str(),
            convert_type(left, left->lexinfo),
            give_name(left->children[0],*left->children[0]->lexinfo),
            give_name(right,*right->lexinfo));
                        break;
         case NUMBER:
         case TOK_CHARCON:
         case TOK_STRINGCON:
         
         plzwork = *right->lexinfo;
         fprintf (oil_output, "%s%s %s = %s;\n", _depth.c_str(),
            convert_type(left, left->lexinfo),
            give_name(left->children[0],*left->children[0]->lexinfo),
            plzwork.c_str());
                        break;
         case TOK_CHAR:  
         fprintf (oil_output, "%s%s %s = %s (%s);\n", _depth.c_str(),
            convert_type(left, left->lexinfo),
            give_name(left->children[0],*left->children[0]->lexinfo),
            give_name(right,*right->lexinfo),
            give_name(right->children[0],
               *right->children[0]->lexinfo));
                        break;
         case TOK_NEWARRAY:     write_new(right, depth);
         plzwork = *left->children[1]->lexinfo;
         fprintf (oil_output, "%s* %s = p%zd;\n",
            convert_type(left->children[0],left->children[0]->lexinfo),
            plzwork.c_str(), reg_counter-1);
                           break;
         case TOK_NEWSTRING:
         case TOK_NEW:      write_new(right, depth);
         plzwork = *left->children[0]->lexinfo;
         fprintf (oil_output, "%s %s = p%zd;\n",
            convert_type(left, left->lexinfo),
            plzwork.c_str(), reg_counter-1);
                        break;
         case '!':   
         fprintf (oil_output, "%s%s %s = !%s;\n", _depth.c_str(),
            convert_type(left, left->lexinfo),
            give_name(left->children[0],*left->children[0]->lexinfo),
            give_name(right->children[0],
               *right->children[0]->lexinfo));
                     break;
         case '+':
         case '-':      if(right->children.size() == 1){
         plzwork = *right->lexinfo;
         fprintf (oil_output, "%s%s %s = %s%s;\n", _depth.c_str(),
            convert_type(left, left->lexinfo),
            give_name(left,*left->lexinfo), plzwork.c_str(),
            give_name(right->children[0],
               *right->children[0]->lexinfo));
                        break;
                     }
         case '/':
         case '*': 
         fprintf (oil_output, "%s%s %s%zd = ", _depth.c_str(),
            convert_type(left, left->lexinfo),
            make_type_reg(*left->lexinfo).c_str(), reg_counter++);
         write_expression(right, depth);
         fprintf (oil_output, ";\n");
         fprintf (oil_output, "%s%s %s = %s%zd;\n", _depth.c_str(),
            convert_type(left, left->lexinfo),
            give_name(left->children[0],
               *left->children[0]->lexinfo),
               make_type_reg(*left->lexinfo).c_str(), reg_counter-1);
                     break;
         default:    
         fprintf (oil_output, "%s%s %s%zd = ", _depth.c_str(),
            convert_type(left, left->lexinfo),
            make_type_reg(*left->lexinfo).c_str(), reg_counter++);
         write_call(right, depth);
         fprintf (oil_output, ";\n");
         fprintf (oil_output, "%s%s %s = %s%zd;\n", _depth.c_str(),
            convert_type(left, left->lexinfo),
            give_name(left->children[0],
               *left->children[0]->lexinfo),
               make_type_reg(*left->lexinfo).c_str(), reg_counter-1);
                     break;
      }
   }else{
      switch (right->symbol){
         case TOK_IDENT:  
         fprintf (oil_output, "%s%s = %s;\n", _depth.c_str(),
            give_name(left, *left->lexinfo),
            give_name(right, *right->lexinfo));
                        break;
         case NUMBER:
         case TOK_CHARCON:
         case TOK_STRINGCON:
         case TOK_CHAR:  
         fprintf (oil_output, "%s%s = %s (%s);\n", _depth.c_str(),
            give_name(left, *left->lexinfo),
            give_name(right, *right->lexinfo),
            give_name(right->children[0],
               *right->children[0]->lexinfo));
                        break;
         case TOK_NEWARRAY:     write_new(right, depth);
         plzwork = *left->lexinfo;
         fprintf (oil_output, "%s = p%zd;\n", plzwork.c_str(),
            reg_counter-1);
                           break;
         case TOK_NEWSTRING:
         case TOK_NEW:      write_new(right, depth);
         plzwork = *left->lexinfo;
         fprintf (oil_output, "%s = p%zd;\n", plzwork.c_str(),
            reg_counter-1);
                        break;
         case '!':  
         plzwork = *right->lexinfo;
         fprintf (oil_output, "%s%s = %s%s;\n", _depth.c_str(),
            give_name(left, *left->lexinfo), plzwork.c_str(),
            give_name(right->children[0],
               *right->children[0]->lexinfo));
                     break;
         case '+':
         case '-':      if(right->children.size() == 1){
         plzwork = *right->lexinfo;
         fprintf (oil_output, "%s%s = %s%s;\n", _depth.c_str(),
            give_name(left, *left->lexinfo), plzwork.c_str(),
            give_name(right->children[0],
               *right->children[0]->lexinfo));
                        break;
                     }
         case '/':
         case '*':  
         fprintf (oil_output, "%sint %s%zd = ", _depth.c_str(),
            make_type_reg(*left->lexinfo).c_str(), reg_counter++);
         write_expression(right, depth);
         fprintf (oil_output, "%s%s = %s%zd;\n", _depth.c_str(),
            give_name(left, *left->lexinfo), 
            make_type_reg(*left->lexinfo).c_str(), reg_counter-1);
                     break;
         default:    
         plzwork = *left->lexinfo;
         fprintf (oil_output, "%s%s %s%zd;\n", _depth.c_str(),
            plzwork.c_str(), 
            make_type_reg(*left->lexinfo).c_str(), reg_counter++);
         write_call(right, depth);
         fprintf (oil_output, ";\n");
         fprintf (oil_output, "%s%s = %s%zd;\n", _depth.c_str(),
            give_name(left, *left->lexinfo), 
            make_type_reg(*left->lexinfo).c_str(), reg_counter-1);
                     break;
      }
   }
}

void oil_stuff(astree* node,int depth, astree* extra){
   string _depth = string(depth * 3, ' ');
   switch (node->symbol){
      case TOK_BLOCK:    for (astree* child: node->children){
                        oil_stuff(child, depth+1, extra);
                     }
                     break;
      case TOK_WHILE:    write_while(node, 0);
                     break;
      case TOK_IF:      write_conditional(node->children[0]);
      fprintf (oil_output, "if(!b%zdgoto fi_%zd_%zd_%zd;\n",
         reg_counter-1, node->lloc.filenr, node->lloc.linenr,
         node->lloc.offset);
      oil_stuff(node->children[1], depth, extra);
      fprintf (oil_output, "fi_%zd_%zd_%zd:;\n",
            node->lloc.filenr, node->lloc.linenr, node->lloc.offset);
                     break;
      case '=':         write_eq(node, depth);
                     break;
      case TOK_RETURN:   write_return(node, depth);
                     break;
      case TOK_PROTOTYPE: break;
      case TOK_CALL:     write_call(node, depth);
      fprintf (oil_output, "; \n");
                     break;
      default:      
      //plzwork = parser::get_tname(node->symbol);
      fprintf(oil_output,"%s%s\n", _depth.c_str(), 
         parser::get_tname(node->symbol));
                     break;
   }
}

void write_function(astree* node, int depth){
   astree* return_type = node->children[0];
   astree* name = return_type->children[0];
   name->symbol = TOK_FUNCTION;
   fprintf(oil_output,"%s %s (\n",
      convert_type(return_type, return_type->lexinfo),
      give_name(name, *name->lexinfo));
   depth++; int next = 2;
   astree* paramlist = node->children[1];
   if(paramlist->symbol == TOK_PARAMLIST){
      for (size_t i = 0; i < paramlist->children.size(); i++){
         astree* param_type = paramlist->children[i];
         astree* param = param_type->children[0];
         fprintf(oil_output,"%s %s (\n",
            convert_type(param_type, param_type->lexinfo),
            give_name(param, *param->lexinfo));
         if(i+1 != paramlist->children.size())
            fprintf(oil_output, ",\n");
      }
   }else{
      next = 1;
   }
   fprintf(oil_output, ")\n{\n");
   astree* block = node->children[next];
   oil_stuff(block, depth, nullptr);
   fprintf(oil_output, "}\n");
}

void write_oil(astree* root, int depth){
   for (astree* child: root->children){
      if(child->symbol == TOK_STRUCT)
         write_struct(child, depth);
   }
   for (astree* child: root->children){
      write_strconst(child, depth);
   }
   for (astree* child: root->children){
      if(child->symbol == '='
         && child->children[0]->children.size() != 0){
         astree* type = child->children[0];
         astree* declid = type->children[0];
         if(declid->symbol == TOK_DECLID
               && type->symbol != TOK_STRING){
            fprintf(oil_output, "%s %s;\n", 
               convert_type(type, nullptr), 
               give_name(type, *declid->lexinfo));
         }
      }
   }
   for (astree* child: root->children){
      if(child->symbol == TOK_FUNCTION){
         write_function(child, depth);
      }
   }
   fprintf(oil_output, "void __ocmain (void)\n{\n");
   for (astree* child: root->children){
      if(child->symbol != TOK_FUNCTION
         && child->symbol != TOK_STRUCT)
         oil_stuff(child, 1, nullptr);
   }
   fprintf(oil_output, "}\nend\n");
   
}

