//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef SMALLSHELLHEADERS_H
#define SMALLSHELLHEADERS_H

//TODO: 'global' files here V

#include "SmashUtil.h"
using namespace SmashUtil;

//TODO: add forwarad declarations here V

class SmallShell;
class Command;

//TODO: add here external libraries V

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
#include <sys/syscall.h>

//TODO: add macros here V

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define MAX_DIR_LENGTH (COMMAND_MAX_LENGTH)

using std::string;
using std::endl;
using std::cout;
using std::cerr;
using std::cin;
using std::istringstream;

typedef std::vector<std::string> argv;
typedef int open_flag;

#define STRINGS_EQUAL(A, B) ((A) == (B))
#define COPY_CHAR_ARR(A, B) (while (*A++ = *B++)) // inline void strcopy(char* destination, char* origin){while(*destination++ = *origin++);}

#define DEBUG_MODE true
#if DEBUG_MODE
#define FOR_DEBUG_MODE(CODE_CONTENTS) CODE_CONTENTS
#else
#define FOR_DEBUG_MODE(CODE_CONTENTS)
#endif

#define Block_until_the_child_terminates 0
#define check_if_process_finished_without_blocking WNOHANG

#define O_RDONLY (0x0000)
#define O_WRONLY (0x0001)
#define O_RDWR   (0x0002)
#define O_CREAT  (0x0200)
#define O_TRUNC  (0x0400)
#define O_APPEND (0x0008)
#define ERR_ARG (-1)

#define SYSTEM_CALL_ERROR (-1)
#define SYSTEM_CALL_FAILED(SYSTEM_CALL) (SYSTEM_CALL == SYSTEM_CALL_ERROR)

#define DO_SYS(x) \
do { \
if ((x) == -1) { \
perror(#x); \
exit(1); \
} \
} while (0)


//TODO: add here project files headers V

#include "Commands.h"
#include "BuiltInCommands.h"
#include "EXternalCommands.h"
#include "SpecialCommands.h"
#include "CommandFactories.h"
#include "JobList.h"
#include "AliasManager.h"
#include "SmallShell.h"

//TODO: add string definitions here V

const char *ChangeDirCommand::TOO_MANY_ARGS = "smash error: cd: too many arguments";
const char *ChangeDirCommand::OLD_PWD_NOT_SET = "smash error: cd: OLDPWD not set";
const char *ForegroundCommand::INVALID_SYNTAX_MESSAGE = "smash error: fg: invalid arguments";
const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_1 = "smash error: fg: job-id ";
const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_2 = " does not exist";
const char *ForegroundCommand::NO_JOBS_MESSAGE = "smash error: fg: jobs list is empty";

//TODO: declarations for easier use here V

// Global alias for the singleton instance
SmallShell &SHELL_INSTANCE = SmallShell::getInstance();


#endif //SMALLSHELLHEADERS_H
