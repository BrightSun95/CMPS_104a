//*****************************************************
//*  Nicholas Pappas
//*  Riley Honbo
//*  nhpappas@ucsc.edu
//*  rhonbo@ucsc.edu
//*  CMPS 104a w/ Mackey
//*  Assignment: Project 5
//*****************************************************

#ifndef __CODE_GENERATOR_H__
#define __CODE_GENERATOR_H__

#include <iostream>
#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

#include "lyutils.h"
#include "auxlib.h"
#include "astree.h"

using namespace std;
extern FILE* oil_output;

const char* give_name(astree* node, string current_name);

void oil_stuff(astree* node, int depth, astree* extra);

void write_oil(astree* root, int depth);

#endif

