#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
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

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

Command* BuiltInCommandFactory::factoryHelper(char **args) {
  // your implementation here
  inline char* comand = args[0];
  if (strcmp(comand, "chprompt") == 0) {
    // handle chprompt
  } else if (strcmp(comand, "showpid") == 0) {
      // handle showpid
  } else if (strcmp(comand, "pwd") == 0) {
      // handle pwd
  } else if (strcmp(comand, "cd") == 0) {
      // handle cd
  } else if (strcmp(comand, "jobs") == 0) {
      // handle jobs
  } else if (strcmp(comand, "fg") == 0) {
      // handle fg
  } else if (strcmp(comand, "quit") == 0) {
      // handle quit
  } else if (strcmp(comand, "kill") == 0) {
      // handle kill
  } else if (strcmp(comand, "alias") == 0) {
      // handle alias
  } else if (strcmp(comand, "unalias") == 0) {
      // handle unalias
  } else if (strcmp(comand, "unsetenv") == 0) {
      // handle unsetenv
  } else if (strcmp(comand, "watchproc") == 0) {
      // handle watchproc
  } else {
      // unknown command
      return nullptr;
  }
}

Command* ExternalCommandFactory::factoryHelper(char **args) {
  // your implementation here
  inline char* comand = args[0];

}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:
  /*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)

    // TODO: command pre - proccesing
    bool backGroundCommand = _isBackgroundComamnd(cmd_line);
    char *non_const_cmd = cmd_line; // TODO: find out how tf to duplicate a char*, only do it for trimming and to get rid of this fkin const
    if (backGroundCommand){
      _removeBackgroundSign(cmd_line);
    }
    _trim(non_const_cmd); // FIXME: same here
    // TODO: command pre - proccesing



}