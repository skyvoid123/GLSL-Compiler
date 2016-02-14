#ifndef _H_symtab
#define _H_symtab
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>
using namespace std;

class Type;
class Decl;
class Symtab {
    protected:
        vector<map<Decl*, Type*>*> *table;
        int levelNumber;
    public:
        Symtab();
        int getLevelNumber();
        void enterScope();
        bool insert(pair<Decl*, Type*>);
        bool find(Decl*, int);
        bool find(Decl*);
        void exitScope();
        void printTable(int);
        void printTable();
};
#endif
