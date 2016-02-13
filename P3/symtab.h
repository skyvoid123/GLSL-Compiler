#ifndef _H_symtab
#define _H_symtab
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
using namespace std;

class Symtab {
    protected:
        vector<map<string, string>*> *table;
        int levelNumber;
    public:
        Symtab();
        int getLevelNumber();
        void enterScope();
        bool insert(pair<string,string>);
        bool find(string);
        void exitScope();
        void printTable(int);
        void printTable();
};
#endif
