#include "errors.h"
#include "symtab.h"

Symtab::Symtab() {
    table = new vector<map<string, string>*>();
    levelNumber = 0;
}

int Symtab::getLevelNumber() {
    return levelNumber;
}

void Symtab::enterScope() {
    table->push_back(new map<string,string>());
    levelNumber++;
}

bool Symtab::insert(pair<string,string> var) {
    if (levelNumber <= 0) {
        cout << "No Scope" << endl;
        return false;
    }
    pair<map<string, string>::iterator, bool> ret;
    ret = table->at(levelNumber - 1)->insert(var);
    //TODO: declaration conflict
    return ret.second;
}

bool Symtab::find(string var) {
    for (int i = levelNumber - 1; i >= 0; i++) {
        if (table->at(i)->count(var) == 1)
            return true;
    }
    //TODO: No declaration Error
    return false;
}

void Symtab::exitScope() {
    table->pop_back();
    levelNumber--;
}
