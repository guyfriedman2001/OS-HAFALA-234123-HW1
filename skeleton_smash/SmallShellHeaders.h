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
// for std::from_chars
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
#include <sys/stat.h>
#include <pwd.h>

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

//FIXME: flip flag and test before submission!
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

//TODO: maybe remove DO_SYS
#define DO_SYS(SYSTEM_CALL, ERROR_STRING) \
do { \
if ((SYSTEM_CALL) == -1) { \
perror(ERROR_STRING); \
exit(1); \
} \
} while (0)

#define TRY_SYS(SYSTEM_CALL, ERROR_STRING) \
do { \
if ((SYSTEM_CALL) == -1) { \
perror(ERROR_STRING); \
} \
} while (0)

//I want the functionality of TRY_SYS without changing existing code and with easier use for smash calls
#define TRY_SYS2(SYSTEM_CALL, SYS_CALL_NAME) \
TRY_SYS(SYSTEM_CALL, ("smash error: " SYS_CALL_NAME " failed"))




//TODO: add here project files headers V

#include "Commands.h"
#include "BuiltInCommands.h"
#include "ExternalCommands.h"
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

const char* SIGKILL_STRING_MESSAGE_1 = "smash: process ";
const char* SIGKILL_STRING_MESSAGE_2 = " was killed";

const char* DiskUsageCommand::TOO_MANY_ARGS = "smash error: du: too many arguments";
const char* DiskUsageCommand::DIRECTORY_DOESNT_EXIST_1 = "smash error: du: directory ";
const char* DiskUsageCommand::DIRECTORY_DOESNT_EXIST_2 = " does not exist";
const char* DiskUsageCommand::TOTAL_DISK_USAGE = "Total disk usage: ";

const char* WatchProcCommand::INVALID_ARGUMENTS = "smash error: watchproc: invalid arguments";
const char* WatchProcCommand::PID_DOESNT_EXIST_1 = "smash error: watchproc: pid ";
const char* WatchProcCommand::PID_DOESNT_EXIST_2 = "does not exist";

const char* UnSetEnvCommand::NOT_ENOUGH_ARGUMENTS = "smash error: unsetenv: not enough arguments";
const char* UnSetEnvCommand::VARIABLE_DOESNT_EXIST_1 = "smash error: unsetenv: ";
const char* UnSetEnvCommand::VARIABLE_DOESNT_EXIST_2 = "does not exist";

const char* UnAliasCommand::NOT_ENOUGH_ARGUMENTS = "smash error: unalias: not enough arguments";
const char* UnAliasCommand::ALIAS_DOESNT_EXIST_1 = "smash error: unalias: ";
const char* UnAliasCommand::ALIAS_DOESNT_EXIST_2 = "does not exist";

const char* AliasCommand::INVALID_FORMAT = "smash error: alias: invalid alias format";
const char* AliasCommand::ALIAS_EXISTS_1 = "smash error: alias: ";
const char* AliasCommand::ALIAS_EXISTS_2 = " already exists or is a reserved command";

const char* KillCommand::INVALID_ARGUMENTS = "smash error: kill: invalid arguments";
const char* KillCommand::JOB_DOESNT_EXIST_1 = "smash error: kill: job-id ";
const char* KillCommand::JOB_DOESNT_EXIST_2 = " does not exist";

const char* QuitCommand::SENDING_SIGKILL = "smash: sending SIGKILL signal to ";

//TODO: declarations for easier use here V

// Global alias for the singleton instance
SmallShell &SHELL_INSTANCE = SmallShell::getInstance();


#endif //SMALLSHELLHEADERS_H
