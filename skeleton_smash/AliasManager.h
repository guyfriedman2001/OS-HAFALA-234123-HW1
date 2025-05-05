//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef ALIASMANAGER_H
#define ALIASMANAGER_H
#include "SmallShellHeaders.h"


class AliasManager {
    std::unordered_map<string, string> aliases;

public:

    argv uncoverAlias(const argv& original);
    void addAlias(const string& newAliasName, string args);
    void removeAlias(const string& aliasToRemove);
    bool isReserved(const string& newAliasName) const;
    bool doesExist(const string& newAliasName) const;
    bool isSyntaxValid(const string& newAliasName) const;
    void printAll() const;
};



#endif //ALIASMANAGER_H
