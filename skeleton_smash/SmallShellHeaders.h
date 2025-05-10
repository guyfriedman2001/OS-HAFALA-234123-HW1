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
//#include "Commands.h"
#include <string>
#include <set>
#include <cstdlib> // for getenv
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>	
#include <net/if.h>	
#include <netinet/in.h>	
#include <arpa/inet.h>	

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



// ============= TODO: add here typedefs =============
typedef std::vector<std::string> argv;
typedef int open_flag;
typedef int fd_location;
typedef std::shared_ptr<Command> cmdp;
typedef int PIPE;

//#define NDEBUG

// ============= TODO: add here permanent disabling flags / switches =============
#ifndef NDEBUG
#define DEBUG_MODE false  //FIXME: switch flag to false, and test before submission! (or simply compile using -DNDEBUG flag)
#else // NDEBUG
#define DEBUG_MODE false //DONT CHANGE THIS LINE! AUTOMATIC DEBUG TURN OFF FOR -DNDEBUG COMPILATION!
#endif // NDEBUG
#define UNFOUND_COMMAND_HANDLED_AUTOMATICALLY true //refers to the case that an invalid command was given and smash tried to execute it externally
#define PIPE_CHANGES_DADDYS_FD false //refers to the case that pipe would change the fd in the original daddy process directly (without using FDManager)
#define PIPES_SHOULD_ONLY_FORK_ONCE true //refers to the case that pipes should only execute in the foreground, not background
#define NEED_TO_HANDLE_ALL_SIGNALS false //refers to whether all signals should be handled; when set to false only ctr c (sigint) would be handled in a non default way
#define JOBS_PRINTS_COMMAND_BEFORE_ALIASES true //refers to whether jobs prints the original given command or the command after the aliases were uncovered
#define REDIRECTION_COMMMANDS_CAN_GET_SIGNALS false
#define PERMANENTLY_DISABLE_SIGNALS_H true

// ============= TODO: add here #define debug utilities and functions =============
#if !DEBUG_MODE //this would make sure that DNDEBUG flag would be defined to take off the asserts, without having to change the compile command
#ifndef NDEBUG
#define NDEBUG
#endif //ifndef NDEBUG
#endif //if !DEBUG_MODE


//this function automatically removes the CODE_CONTENTS if DEBUG_MODE is off
#if DEBUG_MODE
#define FOR_DEBUG_MODE(CODE_CONTENTS) CODE_CONTENTS
#else
#define FOR_DEBUG_MODE(CODE_CONTENTS)
#endif

#define PRINT_DEBUG_MODE(STRING) FOR_DEBUG_MODE(cout << "(PRINT_DEBUG_MODE)  "<< STRING << endl)
#define PRINT_DEBUG_MODE_CERR(STRING) FOR_DEBUG_MODE(cerr << "(PRINT_DEBUG_MODE_CERR)  "<< STRING << endl)
#define PRINT_DEBUG_MODE_CERR_2(STRING) (PRINT_DEBUG_MODE_CERR(STRING)FOR_DEBUG_MODE(;))


#define PERRORF(fmt, ...) \
fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, strerror(errno))

#define PERROR_FOR_DEBUG(fmt, ...) FOR_DEBUG_MODE(PERRORF(fmt, __VA_ARGS__);)

#define MARK_FOR_DEBUGGING_PERROR \
FOR_DEBUG_MODE( do { PERRORF("(FOR_DEBUG_MODE(PERRORF)) error in file: %s, line: %d\n", __FILE__, __LINE__); } while (0); )

// ============= TODO: add here #define utilities and functions =============
#define STRINGS_EQUAL(A, B) ((A) == (B))
#define COPY_CHAR_ARR(A, B) (while (*A++ = *B++)) // inline void strcopy(char* destination, char* origin){while(*destination++ = *origin++);}
#define MAX(A,B) ((A) > (B) ? (A) : (B)) //return maximum between two numbers
#define SYSTEM_CALL_FAILED(SYSTEM_CALL) (SYSTEM_CALL == SYSTEM_CALL_ERROR) //return true if system call failrd (on system calls that return -1 on failure only, does not handle system calls that return 0 or nullptr on failure)

//try a system call, and on failure (for system calls that return -1 on failure) print ERROR_STRING to stderr and exit
#define DO_SYS(SYSTEM_CALL, ERROR_STRING) \
do { \
if (SYSTEM_CALL_FAILED(SYSTEM_CALL)) { \
perror(ERROR_STRING); \
exit(1); \
} \
} while (0)

//same as DO_SYS except without exiting
#define TRY_SYS(SYSTEM_CALL, ERROR_STRING) \
do { \
if (SYSTEM_CALL_FAILED(SYSTEM_CALL)) { \
perror(ERROR_STRING); \
FOR_DEBUG_MODE(PERRORF("(FOR_DEBUG_MODE(PERRORF)) error in file: %s, line: %d\n",__FILE__,__LINE__);) \
} \
} while (0)

//I want the functionality of TRY_SYS without changing existing code and with easier use for smash calls
#define TRY_SYS2(SYSTEM_CALL, SYS_CALL_NAME) \
TRY_SYS(SYSTEM_CALL, ("smash error: " SYS_CALL_NAME " failed"))

#define IF_THEN_ELSE_LOGICALL(FLAG, RUN_IF_TRUE, RUN_IF_FALSE) \
    do {                                                \
        if (FLAG) {                                     \
            RUN_IF_TRUE;                               \
        } else {                                        \
            RUN_IF_FALSE;                              \
        }                                               \
    } while (0)

#define TRY_SYS3(VAR, EXPR, NAME)                  \
do {                                                     \
    VAR = (EXPR);                                        \
    if (SYSTEM_CALL_FAILED(VAR)) {                       \
        perror("smash error: " NAME " failed");          \
        FOR_DEBUG_MODE(PERRORF("error in file: %s, line: %d\n",__FILE__,__LINE__);) \
    }                                                    \
} while (0)
/* //i cant get this shit to work blyat
#define IF_THEN_ELSE_PREPROCESSOR(FLAG, CODE_IF_TRUE, CODE_IF_FALSE) \
    #if FLAG                                            \
        CODE_IF_TRUE                                    \
    #else                                              \
        CODE_IF_FALSE                                   \
    #endif

#define PREPROCESSOR_COMMA ,
*/


// ============= TODO: ADD HERE TEMPORAIRLY DISABLING FLAGS =============
#define ONLY_FOR_DEBUG(BOOL) (DEBUG_MODE&&BOOL)
#define TEMPORAIRLY_DISABLE_CTRL_HANDLER ONLY_FOR_DEBUG(false)
//#define temporairly_disable_kill_all_jobs ONLY_FOR_DEBUG(true)
//#define temporairly_disable_removeFinishedJobs ONLY_FOR_DEBUG(true)
#define messing_around ONLY_FOR_DEBUG(true)




// ============= TODO: add here 'magic number' definitions =============

//standard in/out/error file descriptor indexes
#define STDIN_FILE_NUM 0
#define STDOUT_FILE_NUM 1
#define STDERR_FILE_NUM 2

//open flag for permissions for new files
#define RWXRWXRWX (0777)
#define OPEN_IN_GOD_MODE RWXRWXRWX
#define RW_R__R__ (0644)
#define RWXR__R__ (0744)

//pipe definitions for easier and more readable pipe handling
#define PIPE_SIZE (2)
#define PIPE_READ (0)
#define PIPE_WRITE (1)

//wait flags
#define Block_until_the_child_terminates 0
#define check_if_process_finished_without_blocking WNOHANG
#define wait_but_can_still_get_ctrl_c WUNTRACED

//open file flags
//#define O_RDONLY (0x0000)
//#define O_WRONLY (0x0001)
//#define O_RDWR   (0x0002)
//#define O_CREAT  (0x0200)
//#define O_TRUNC  (0x0400)
//#define O_APPEND (0x0008)

//general magic numbers
#define ERR_ARG (-1)
#define SYSTEM_CALL_ERROR (-1)

//small shell handling external processes in the foreground
#define NO_CURRENT_EXTERNAL_FOREGROUND_PROCESS_PID (-1)









//TODO: add here project files headers V

//#include "Commands.h"
//#include "BuiltInCommands.h"
//#include "ExternalCommands.h"
//#include "SpecialCommands.h"
//#include "CommandFactories.h"
//#include "JobList.h"
//#include "AliasManager.h"
//#include "SmallShell.h"

//TODO: add string definitions here V







//TODO: declarations for easier use here V

// Global alias for the singleton instance
//SmallShell &SHELL_INSTANCE = SmallShell::getInstance();


#endif //SMALLSHELLHEADERS_H
