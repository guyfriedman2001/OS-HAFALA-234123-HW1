//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef SMASHUTIL_H
#define SMASHUTIL_H
#include <climits>
#include <set>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>




const std::string WHITESPACE = " \n\r\t\f\v";
using std::string;
typedef std::vector<std::string> argv;


namespace  SmashUtil {

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

inline string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

inline string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

inline string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

inline argv parseCommandLine(const string& cmd_s) {
    std::istringstream iss(cmd_s);
    argv result;
    string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

/**
 * Parses a command-line string into individual whitespace-separated arguments
 * and stores them as C-style strings in the provided `args` array.
 *
 * @param cmd_line A null-terminated string representing the full command line to be parsed.
 * @param args An array of `char*` pointers where each parsed argument will be stored.
 *             The array must be pre-allocated with enough space for all expected arguments.
 *
 * @return The number of arguments parsed (i.e., how many entries in `args` are filled).
 *
 * The function performs the following steps:
 * - Trims leading and trailing whitespace from `cmd_line`.
 * - Splits the string into tokens using whitespace as the delimiter.
 * - For each token:
 *   - Allocates memory for a null-terminated C-style string.
 *   - Copies the token into the newly allocated memory.
 *   - Stores the pointer in the `args` array.
 * - The array is terminated with a `NULL` pointer after the last argument.
 *
 * Example:
 *   const char* input = "ls -la /home/user";
 *   char* args[10];
 *   int count = _parseCommandLine(input, args);
 *   // args = {"ls", "-la", "/home/user", NULL}
 *   // count = 3
 *
 * Notes:
 * - The caller is responsible for freeing the memory allocated for each entry in `args`.
 * - The function assumes that `args` has enough space to hold all tokens and the terminating `NULL`.
 */
inline int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    std::memset(args[i], 0, s.length() + 1);
    std::strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}


/**
 * @brief free memory malloc'ed by _parseCommandLine
 *
 * @arg args Arguments parsed by _parseCommandLine.
 * @arg args_num Number of arguments returned by _parseCommandLine.
 *
 * @note Call this function after processing `args` to prevent memory leaks.
 *
 * @note The function sets each element of `args` to `NULL` after freeing it, to avoid
 *       potential dangling pointer issues.
 */
inline void commandDestructor(char **args, int args_num)
{
  for (int i = 0; i < args_num; ++i)
  {
    if (args[i] != NULL)
    {
      free(args[i]);  // Free the memory allocated for each argument
      args[i] = NULL; // Optional: Set the element to NULL to avoid dangling pointers
    }
  }
}

inline bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

inline void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

inline bool isBuiltInCommand(const std::string &cmd)
{
  static const std::set<std::string> builtins = {
      "cd", "pwd", "jobs", "fg", "bg", "kill", "quit", "chprompt", "showpid", "set", "unset", "alias", "unalias"};
  return builtins.find(cmd) != builtins.end();
}

inline bool isExternalComamnd(const char *cmd_line)
{
  // TODO: create
  return false;
}

inline bool isWildCard(const char *cmd_line)
{
  return std::strchr(cmd_line, '?');
}

inline bool isCompleExternalCommand(const char *cmd_line)
{
  return (isWildCard(cmd_line) || std::strchr(cmd_line, '*'));
}

inline bool isPipeCommand(const char *cmd_line)
{
  return (std::strchr(cmd_line, '|'));
}

inline bool isInputRedirectionCommand(const char *cmd_line)
{
  return (std::strchr(cmd_line, '<'));
}

inline bool isOutputRedirectionCommand(const char *cmd_line)
{
  return (std::strchr(cmd_line, '>'));
}

inline bool isIORedirectionCommand(const char *cmd_line)
{
  return (isInputRedirectionCommand(cmd_line) || isOutputRedirectionCommand(cmd_line));
}

#include <cassert>
template <typename T>
inline void assert_not_empty(const T &container)
{
  assert(!container.empty());
}

/**
 *
 * @param s string to turn into int
 * @param out location to store int convertion
 * @return true if conversion succesfull, false otherwise
 */
inline bool stringToInt(const std::string &s, int &out) // function from StackOverflow, nned to make sure that it works
{
  char *end = nullptr;
  errno = 0; // reset errno before call
  long val = std::strtol(s.c_str(), &end, 10);

  if (errno != 0 || end != s.c_str() + s.size())
  {
    // invalid number or not fully consumed
    return false;
  }

  // check overflow for int range
  if (val < INT_MIN || val > INT_MAX)
  {
    return false;
  }

  out = static_cast<int>(val);
  return true;
}

}



#endif //SMASHUTIL_H
