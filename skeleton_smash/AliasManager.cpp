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
    return true;
  }
  return false;
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

argv AliasManager::uncoverAlias(const argv& original)
{
  /*argv uncoveredArgs;

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
  }
  return uncoveredArgs;*/

  argv uncoveredArgs;

  if (original.empty() || original[0] == "unalias" || original[0] == "alias") {
    return original;
  }

  for (const string& arg : original) {
    if (aliases.find(arg) != aliases.end()) {
      istringstream iss(aliases[arg]);
      string word;
      while (iss >> word) {
        argv tokens = tokenizeWithSpecials(word);
        uncoveredArgs.insert(uncoveredArgs.end(), tokens.begin(), tokens.end());
      }
    } else {
      argv tokens = tokenizeWithSpecials(arg);
      uncoveredArgs.insert(uncoveredArgs.end(), tokens.begin(), tokens.end());
    }
  }

  return uncoveredArgs;
}

argv AliasManager::tokenizeWithSpecials(const string& token) {
  argv result;
  string current;

  for (size_t i = 0; i < token.size(); ++i) {
    char c = token[i];

    if ((c == '>' || c == '<') && i + 1 < token.size() && token[i + 1] == c) {
      if (!current.empty()) {
        result.push_back(current);
        current.clear();
      }
      result.push_back(string(2, c)); 
      ++i; 
    }
    else if (c == '>' || c == '<' || c == '|' || c == '&') {
      if (!current.empty()) {
        result.push_back(current);
        current.clear();
      }
      result.push_back(string(1, c));
    }
    else {
      current += c;
    }
  }

  if (!current.empty()) {
    result.push_back(current);
  }

  return result;
}
