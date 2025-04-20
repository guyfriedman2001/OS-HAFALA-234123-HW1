
#include "SmallShellHeaders.h"
#include "SpecialCommands.h"

#if 1
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cassert>
#include <charconv>  // for std::from_chars
#include <fcntl.h>  // for open()
#include <unistd.h> //for close()
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <regex>
#include "Commands.h"
#include <string>
#include <set>
#include <cstdlib> // for getenv
#endif

class SmallShell;

// #define STRINGS_EQUAL(A, B) (strcmp((A), (B)) == 0)

// Global alias for the singleton instance
// SmallShell &SHELL_INSTANCE = SmallShell::getInstance();

using namespace std;
using namespace SmashUtil;

pid_t Command::getPID()
{
  return getpid();
}

CommandNotFound::CommandNotFound(const argv &args)
{
  // TODO:
}

CommandNotFound::CommandNotFound(const argv &args, const char *cmd_line)
{
}

void CommandNotFound::execute()
{
  // TODO:
}




