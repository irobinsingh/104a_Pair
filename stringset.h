
// Assignment 1 CS 104a
// Authors: Konstantin Litovskiy and Gahl Levy
// Users Names: klitovsk and grlevy

#ifndef __STRINGSET__
#define __STRINGSET__

#include <string>
#include <unordered_set>
using namespace std;

#include <stdio.h>

#include "auxlib.h"

const string* intern_stringset (const char*);

void dump_stringset (FILE*);

RCSH("$Id: stringset.h,v 1.5 2013-09-23 14:16:09-07 - - $")
#endif
