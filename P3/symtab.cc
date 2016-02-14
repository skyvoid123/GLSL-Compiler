#include "errors.h"
#include "symtab.h"

Symtab::Symtab() {
    table = new vector<map<Decl*, Type*>*>();
    levelNumber = 0;
}

int Symtab::getLevelNumber() {
    return levelNumber;
}

void Symtab::enterScope() {
    table->push_back(new map<Decl*,Type*>());
    levelNumber++;
}

bool Symtab::insert(pair<Decl*,Type*> var) {
    if (levelNumber <= 0) {
        cout << "No Scope" << endl;
        return false;
    }
    cout << "inserting " << var.first << endl;
    if (find(var.first, levelNumber - 1)) {
        cout << "DECLARATION CONLICT" << endl;
        return false;
    }
    pair<map<Decl*, Type*>::iterator, bool> ret;
    table->at(levelNumber - 1)->insert(var);
    return true;
}

bool comparator(Decl* a, Decl* b) {
    return (strcmp(a->getId(),b->getId()) == 0);
}
bool Symtab::find(Decl* var, int x) {
    map<Decl*, Type*> currMap = *table->at(x);
    for (map<Decl*,Type*>::iterator it=currMap.begin();it!=currMap.end();++it) {
        if (comparator(it->first, var)) 
            return true;
    }
    return false;
}

Type* Symtab::findType(Decl* var) {
    for (int i = levelNumber - 1; i >= 0; i++) {
        map<Decl*, Type*> currMap = *table->at(i);
        for (map<Decl*,Type*>::iterator it=currMap.begin();it!=currMap.end();++it) {
            if (comparator(it->first, var)) 
                return it->second;
        }
    }
    return NULL;
}
bool Symtab::find(Decl* var) {
    for (int i = levelNumber - 1; i >= 0; i++) {
        if (find(var, i))
            return true;
    }
    //TODO: No declaration Error
    cout << "NO DECLARATION ERROR" << endl;
    return false;
}

void Symtab::exitScope() {
    table->pop_back();
    levelNumber--;
}

void Symtab::printTable(int x) {
    if (x >= levelNumber) {
        cout << x << " Invalid Level" << endl;
        return;
    }
    map<Decl*, Type*> currMap = *table->at(x);
    if (currMap.empty()) {
        cout << "EMPTY LEVEL" << endl;
        return;
    }
    for (map<Decl*, Type*>::iterator it = currMap.begin(); it!=currMap.end(); ++it)
        cout << it->first << " => " << it->second << endl;
}

void Symtab::printTable() {
    if (table->empty()) {
        cout << "EMPTY TABLE" << endl;
        return;
    }
    for (int i = 0; i < levelNumber; i++) {
        cout << "Printing Level " << i << endl;
        printTable(i);
    }
}
