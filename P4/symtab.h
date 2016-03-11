#ifndef _H_symtab
#define _H_symtab
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>
#include "irgen.h"
#define GLOBAL 1
#define LOCAL 0
#define INVALID -1
#define DEBUG 0
using namespace std;

class Decl;
class Type;

typedef struct container {
    Decl* decl;
    llvm::Value* val;
    int flag;
} container;

class Symtab {
    protected:
        vector<map<string, container>*> *table;
        int levelNumber;
    public:
        Symtab();
        int getLevelNumber();
        void enterScope();
        bool insert(pair<string, container>);
        container find(string, int);
        container find(string);
        void exitScope();
        void printTable(int);
        void printTable();
};
#endif
