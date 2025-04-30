//
// Created by Guy Friedman on 20/04/2025.
//

//#include "AliasManager.h"
#include "AliasManager.h"

#include "SmallShellHeaders.h"

void AliasManager::addAlias(const string &newAliasName, string args)
{
  /*if (isReserved(newAliasName) || doesExist(newAliasName))
  {
    cerr << "smash error: alias: <name> already exists or is a reserved command" << endl;
  }

  if (isSyntaxValid(newAliasName))
  {
    cerr << "smash error: alias: invalid alias format" << endl;
  } */
  aliases.insert(std::make_pair(newAliasName, args));
}

void AliasManager::removeAlias(const string &aliasToRemove)
{
  aliases.erase(aliasToRemove);
}

bool AliasManager::isReserved(const string &newAliasName) const
{
  if (newAliasName == "cd" || newAliasName == "pwd" || newAliasName == "chprompt" || newAliasName == "showpid" ||
      newAliasName == "jobs" || newAliasName == "fg" || newAliasName == "alias" || newAliasName == "quit" || newAliasName == "unalias" ||
      newAliasName == "kill" || newAliasName == "unsetenv" || newAliasName == "watchproc" ||
      newAliasName == "lisidr" || newAliasName == "ls") // TODO maybe add more reserved words
  {
    return false;
  }
  return true;
}

#define ignore_for_now false
#if !ignore_for_now
bool AliasManager::doesExist(const string &newAliasName) const
{
  if (aliases.find(newAliasName) != aliases.end())
  {
    return true;
  }
  return false;
}
#endif

bool AliasManager::isSyntaxValid(const string &newAliasName) const
{
  const std::regex pattern("^alias [a-zA-Z0-9_]+='[^']*'$");
  return std::regex_match(newAliasName, pattern);
}

void AliasManager::printAll() const
{
  for (const auto &alias : aliases)
  {
    cout << alias.first << "=" << "'" << alias.second << "'" << endl;
  }
}

argv AliasManager::uncoverAlias(argv original)
{
  argv uncoveredArgs;

  if (original[0] == "unalias" || original[0] == "alias")
  {
    return original;
  }
  for (int i = 0; i < original.size(); i++)
  {
    if (aliases.find(original[i]) != aliases.end())
    {
      istringstream iss(aliases[original[i]]);
      string word;
      while (iss >> word)
      {
        uncoveredArgs.push_back(word);
      }
    }
    else
    {
      uncoveredArgs.push_back(original[i]);
    }
    return uncoveredArgs;
  }
}
