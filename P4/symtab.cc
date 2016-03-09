#include "errors.h"
#include "symtab.h"

Symtab::Symtab() {
    table = new vector<map<string, container>*>();
    levelNumber = 0;
}

int Symtab::getLevelNumber() {
    return levelNumber;
}

void Symtab::enterScope() {
    table->push_back(new map<string,container>());
    levelNumber++;
}

bool Symtab::insert(pair<string, container> var) {
    if (levelNumber <= 0) {
        cout << "No Scope" << endl;
        return false;
    }
    if (find(var.first, levelNumber - 1).valid != -1) {
        return false;
    }
    table->at(levelNumber - 1)->insert(var);
    return true;
}

container Symtab::find(string var, int x) {
    map<string, container> *currMap = table->at(x);
    map<string, container>::iterator it;
    it = currMap->find(var);
    if (it != currMap->end())
        return it->second;
    container temp;
    temp.valid = -1;
    return temp;
}

container Symtab::find(string var) {
    for (int i = levelNumber - 1; i >= 0; i++) {
        container c = find(var, i);
        if (c.valid != -1)
            return c;
    }
    container temp;
    temp.valid = -1;
    return temp;
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
    map<string, container> currMap = *table->at(x);
    if (currMap.empty()) {
        cout << "EMPTY LEVEL" << endl;
        return;
    }
    for (map<string, container>::iterator it = currMap.begin(); it!=currMap.end(); ++it)
        cout << it->first  << endl;
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
