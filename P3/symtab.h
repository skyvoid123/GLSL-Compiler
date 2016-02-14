#ifndef _H_symtab
#define _H_symtab
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
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
        bool find(Decl*);
        void exitScope();
        void printTable(int);
        void printTable();
};
#endif
