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
    if (ret.second == false)
        cout << "DECLARATION CONFLICT" << endl;
    return ret.second;
}

bool Symtab::find(string var) {
    for (int i = levelNumber - 1; i >= 0; i++) {
        if (table->at(i)->count(var) == 1)
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
    }
    map<string, string> currMap = *table->at(x);
    for (map<string,string>::iterator it = currMap.begin(); it!=currMap.end(); ++it)
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
