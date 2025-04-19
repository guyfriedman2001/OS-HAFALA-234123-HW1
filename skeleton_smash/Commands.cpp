#include <fcntl.h> // for open()
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
#include <cstdlib>  // for getenv

class SmallShell;

//#define STRINGS_EQUAL(A, B) (strcmp((A), (B)) == 0)
#define STRINGS_EQUAL(A, B) ((A) == (B))
#define COPY_CHAR_ARR(A, B) (while (*A++ = *B++)) // inline void strcopy(char* destination, char* origin){while(*destination++ = *origin++);}

#define DEBUG_MODE true
#if DEBUG_MODE
  #define FOR_DEBUG_MODE(CODE_CONTENTS) CODE_CONTENTS
#else
  #define FOR_DEBUG_MODE(CODE_CONTENTS)
#endif

#define FOREGROUND_WAIT_MODIFIER 0
// Global alias for the singleton instance
SmallShell &SHELL_INSTANCE = SmallShell::getInstance();

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
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
int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
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

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
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




bool isBuiltInCommand(const std::string& cmd) {
  static const std::set<std::string> builtins = {
      "cd", "pwd", "jobs", "fg", "bg", "kill", "quit", "chprompt", "showpid", "set", "unset", "alias", "unalias"
  };
  return builtins.find(cmd) != builtins.end();
}

inline bool isExternalComamnd(const char* cmd_line) {
  //TODO: create
  return false;
}

inline bool isWildCard(const char* cmd_line){
  return strchr(cmd_line, '?');
}

inline bool isCompleExternalCommand(const char* cmd_line) {
  return (isWildCard(cmd_line) || strchr(cmd_line, '*'));
}

inline bool isPipeCommand(const char* cmd_line) {
  return (strchr(cmd_line, '|'));
}

inline bool isInputRedirectionCommand(const char* cmd_line) {
  return (strchr(cmd_line, '<'));
}

inline bool isOutputRedirectionCommand(const char* cmd_line) {
  return (strchr(cmd_line, '>'));
}

inline bool isIORedirectionCommand(const char* cmd_line) {
  return (isInputRedirectionCommand(cmd_line) || isOutputRedirectionCommand(cmd_line));
}

template <typename T>
inline void assert_not_empty(const T& container) {
  assert(!container.empty());
}

/**
 *
 * @param s string to turn into int
 * @param out location to store int convertion
 * @return true if conversion succesfull, false otherwise
 */
bool stringToInt(const std::string& s, int& out) //function from StackOverflow, nned to make sure that it works
{
  char* end = nullptr;
  errno = 0; // reset errno before call
  long val = std::strtol(s.c_str(), &end, 10);

  if (errno != 0 || end != s.c_str() + s.size()) {
    // invalid number or not fully consumed
    return false;
  }

  // check overflow for int range
  if (val < INT_MIN || val > INT_MAX) {
    return false;
  }

  out = static_cast<int>(val);
  return true;
}
// TODO: Add your implementation for classes in Commands.h

// ########################## NOTE: AbstractCommand code area V ##########################

//pid_t Command::getPID(){return getpid();}

pid_t Command::getPID()
{
  return getpid();
}


// ########################## NOTE: AbstractCommand code area V ##########################

// ########################## NOTE: CommandFactory code area V ##########################

Command *BuiltInCommandFactory::factoryHelper(argv args, const char* cmd_line)
{
  string& command = args[0];
  if (STRINGS_EQUAL(command, "chprompt"))
  {
    return new ChangePromptCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "showpid"))
  {
    return new ShowPidCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "pwd"))
  {
    return new GetCurrDirCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "cd"))
  {
    return new ChangeDirCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "jobs"))
  {
    return new JobsCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "fg"))
  {
    return new ForegroundCommand(args,  cmd_line);
  }
  else if (STRINGS_EQUAL(command, "quit"))
  {
    return new QuitCommand(args,  cmd_line);
  }
  else if (STRINGS_EQUAL(command, "kill"))
  {
    return new KillCommand(args,  cmd_line);
  }
  else if (STRINGS_EQUAL(command, "alias"))
  {
    return new AliasCommand(args,  cmd_line);
  }
  else if (STRINGS_EQUAL(command, "unalias"))
  {
    return new UnAliasCommand(args,  cmd_line);
  }
  else if (STRINGS_EQUAL(command, "unsetenv"))
  {
    return new UnSetEnvCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "watchproc"))
  {
    return new WatchProcCommand(args, cmd_line);
  }
  else
  { // unknown command
    return nullptr;
  }
}

Command *ExternalCommandFactory::factoryHelper(argv args, const char* cmd_line)
{
  // TODO: your implementation here
  string& command = args[0];
}

/*
void runExternalCommand(const char* cmd_line) {
    pid_t pid = fork();
    
    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return;
    }
    
    if (pid == 0) {
        // Child process

        // Parse the cmd_line into program + arguments
        char cmd_copy[1024];
        strncpy(cmd_copy, cmd_line, sizeof(cmd_copy));
        cmd_copy[sizeof(cmd_copy) - 1] = '\0'; // null-terminate
        
        // Tokenize the command
        std::vector<char*> args;
        char* token = strtok(cmd_copy, " ");
        while (token != nullptr) {
            args.push_back(token);
            token = strtok(nullptr, " ");
        }
        args.push_back(nullptr); // execvp expects a nullptr at the end

        // Execute the command
        if (execvp(args[0], args.data()) == -1) {
            perror("execvp failed");
            exit(1);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // wait for the child
    }
}
*/

Command *SpecialCommandFactory::factoryHelper(argv args, const char* cmd_line)
{
  // TODO: your implementation here
  string& command = args[0];
}

Command *Error404CommandNotFound::factoryHelper(argv args, const char* cmd_line)
{
  return new CommandNotFound(args, cmd_line);
}

// ########################## NOTE: CommandFactory code area ^ ##########################

// ########################## NOTE: SmallShell code area V ##########################

SmallShell::SmallShell()
    : currentPrompt("smash"), promptEndChar(">"), old_path_set(false)
{
}

SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

bool SmallShell::hasOldPath()
{
  return this->old_path_set;
}

string SmallShell::getPreviousPath()
{
  return oldPWD;
}

inline void SmallShell::print_current_path() const
{
  GetCurrDirCommand temp;
  temp.execute();
}

JobsList& SmallShell::getJobsList()
{
  return this->jobs;
}

void SmallShell::print_jobs()
{
  this->jobs.printJobsList();
}

JobsList::JobEntry* SmallShell::getJobById(int jobId)
{
  return this->jobs.getJobById(jobId);
}








/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{
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

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  Command *returnCommand = nullptr;
  argv args = argv(); //FIXME: after we make a function to return argv after aliasing, add call to that function @here


  //char *args_[COMMAND_MAX_ARGS];
  size_t num_args = args.size();//_parseCommandLine(cmd_line, args_); //get num of arguments
  //commandDestructor(args_, num_args);



  if (num_args == 0)
  {
    returnCommand = new EmptyCommand(args,  cmd_line); // TODO: maybe make 'empty command'
  }

  if (returnCommand == nullptr)
  { //try and create a BuiltInCommand command
    BuiltInCommandFactory factory;
    returnCommand = factory.makeCommand(args,  cmd_line);
    //returnCommand = BuiltInCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  { // TODO: might need a bit more logic to decide if a command is just external or special.
    SpecialCommandFactory factory;
    returnCommand = factory.makeCommand(args,  cmd_line);
    //returnCommand = SpecialCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    ExternalCommandFactory factory;
    returnCommand = factory.makeCommand(args,  cmd_line);
    //returnCommand = ExternalCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    Error404CommandNotFound factory;
    returnCommand = factory.makeCommand(args,  cmd_line);
    //returnCommand = Error404CommandNotFound::makeCommand(args, num_args, cmd_line);
  }

  return returnCommand;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)

  /**
  // TODO: command pre - proccesing
  bool backGroundCommand = _isBackgroundComamnd(cmd_line);
  char *non_const_cmd = cmd_line; // TODO: find out how tf to duplicate a char*, only do it for trimming and to get rid of this fkin const
  if (backGroundCommand){
    _removeBackgroundSign(cmd_line);
  }
  _trim(non_const_cmd); // TODO: same here
  // TODO: command pre - proccesing
  */
}

void SmallShell::executeCommand(Command *command)
{
  command->execute();
}

std::string SmallShell::getDefaultPrompt()
{
  return "smash";
}

void SmallShell::changePrompt(std::string nextPrompt)
{
  this->currentPrompt = move(nextPrompt);
}

int SmallShell::getPID()
{
  return getpid();
}

char *SmallShell::loadShellPath(char *buffer_location, size_t buffer_size)
{
  return getcwd(buffer_location, buffer_size);
}

void SmallShell::tryLoadShellPath(char *buffer_location, size_t buffer_size)
{
  if (loadShellPath(buffer_location, buffer_size) == nullptr)
  {
    perror("smash error: getcwd failed");
    FOR_DEBUG_MODE(perror("void SmallShell::tryLoadShellPath(char *buffer_location, size_t buffer_size)\n");)
  }
}

void SmallShell::updateOldPath()
{ // store the current path @oldPWD, to be called before path changing;
  tryLoadShellPath(this->oldPWD, sizeof(this->oldPWD));
  this->old_path_set = true;
}

bool SmallShell::changeShellDirectory(const char *next_dir)
{ // return true on succes, false on failure
  updateOldPath();
  if (chdir(next_dir) != 0)
  {
    // perror("chdir failed");
    FOR_DEBUG_MODE(perror("failure in 'bool SmallShell::changeShellDirectory(const char *next_dir)'\n");)
    return false;
  }
  else
  {
    // printf("chdir sucsess");
    return true;
  }
}

std::string SmallShell::getPrompt()
{
  return this->currentPrompt;
}

std::string SmallShell::getEndStr()
{
  return this->promptEndChar;
}

int SmallShell::waitPID(pid_t pid)
{
  int status;
  waitpid(pid, &status, FOREGROUND_WAIT_MODIFIER);
  return status;
}

int  SmallShell::get_max_current_jobID()
{
  return this->jobs.get_max_current_jobID();
}



// ########################## NOTE: SmallShell code area ^ ##########################

// ########################## NOTE: BuiltInCommand code area V ##########################

CommandNotFound::CommandNotFound(const argv& args)
{
  // TODO:
}


CommandNotFound::CommandNotFound(const argv& args, const char* cmd_line) {

}

void CommandNotFound::execute()
{
  // TODO:
}

ChangePromptCommand::ChangePromptCommand(const argv& args) : nextPrompt((args.size() == 1) ? SmallShell::getDefaultPrompt() : string(args[1])) {
  assert_not_empty(args);
}

ChangePromptCommand::ChangePromptCommand(const argv& args, const char* cmd_line) : ChangePromptCommand(args) {}

void ChangePromptCommand::execute()
{
  SHELL_INSTANCE.changePrompt(this->nextPrompt);
}

ShowPidCommand::ShowPidCommand() : smashPID(SHELL_INSTANCE.getPID()) {}

ShowPidCommand::ShowPidCommand(const argv& args) : ShowPidCommand() {
  assert_not_empty(args);
}

ShowPidCommand::ShowPidCommand(const argv& args, const char* cmd_line)
    : ShowPidCommand(args) {}

void ShowPidCommand::execute()
{
  printf("smash pid is %d", this->smashPID);
}

GetCurrDirCommand::GetCurrDirCommand() //TODO: need to initialise fields? maybe.
{
  SHELL_INSTANCE.tryLoadShellPath(this->current_path, sizeof(this->current_path));
}

GetCurrDirCommand::GetCurrDirCommand(const argv& args) : GetCurrDirCommand() {
  assert_not_empty(args);
}

GetCurrDirCommand::GetCurrDirCommand(const argv& args, const char* cmd_line) : GetCurrDirCommand(args) {}

void GetCurrDirCommand::execute()
{
  FOR_DEBUG_MODE(printf("void GetCurrDirCommand::execute()\n");)
  printf("%s", this->current_path);
}

ChangeDirCommand::ChangeDirCommand(const argv& args)
{
  /** TODO: get the arg, if its empty just need to update the do nothing flag,
            is its just "-" then need to load prev directory from shell to this next dir,
            if prev path not set then need to see how to notice it and set appropriate flag
            if its an addition to a relative path, need to see how to do it
            if its to many arguments, need to set the corresponding flag

            if all flags are to remain 'false' (meaning command is ok), need
            to store the next path on this->next_path
  */
  assert_not_empty(args);
  int given_args = args.size() - 1; //first arg is the call to the function itself, so its accounted for with the -1.
  if (given_args == 0) {
    this->DoNothing = true;
  }
  if (given_args > 1) {
    this->TooManyArgs = true;
  }

  string next_path_arg = args[1];

  if (STRINGS_EQUAL(args[1], "-")) {
    if (SHELL_INSTANCE.hasOldPath()) {
      next_path_arg = SHELL_INSTANCE.getPreviousPath();
    } else {
      this->OldPWDNotSet = true;
    }
  }
  bool command_invalid = (this->DoNothing || this->TooManyArgs || this->OldPWDNotSet);
  if (!command_invalid) {
    strcpy(this->next_path, next_path_arg.c_str());
  }
}

const char* ChangeDirCommand::TOO_MANY_ARGS = "smash error: cd: too many arguments";
const char* ChangeDirCommand::OLD_PWD_NOT_SET = "smash error: cd: OLDPWD not set";

ChangeDirCommand::ChangeDirCommand(const argv& args, const char* cmd_line)
    : ChangeDirCommand(args) {}

void ChangeDirCommand::execute()
{
  if (this->DoNothing)
  { // no args were given.
    return;
  }
  else if (this->TooManyArgs)
  { // too many args were given.
    perror(this->TOO_MANY_ARGS);
    return;
  }
  else if (this->OldPWDNotSet)
  { //tried to load a non existing old path
    perror(this->OLD_PWD_NOT_SET);
    return;
  }
  bool succses = SHELL_INSTANCE.changeShellDirectory(this->next_path);
  if (succses)
  { //print updated path
    SHELL_INSTANCE.print_current_path();
  }
  else
  {
    FOR_DEBUG_MODE(perror("some trouble in 'void ChangeDirCommand::execute()'");)
  }
}

JobsCommand::JobsCommand(const argv& args)
{
  // TODO:
}

JobsCommand::JobsCommand(const argv& args, const char* cmd_line)
    : JobsCommand(args)
{
  // TODO: finish dis
}


void JobsCommand::execute()
{
  // TODO:
}

ForegroundCommand::ForegroundCommand(const argv& args)
{
  assert_not_empty(args);
  bool incorrect_args_ammount = args.size() > 2; //first arg should be "fg" and second (optional) arg should be a specific job ID
  bool explicit_jobID_given = args.size() > 2;
  bool second_arg_is_convertible_to_int;

  if (explicit_jobID_given) {
    second_arg_is_convertible_to_int = stringToInt(args[1],this->jobID); //if second arg was given, make sure that its an int
  } else {
    this->jobID = SHELL_INSTANCE.get_max_current_jobID();
    second_arg_is_convertible_to_int = true;
  }

  if (incorrect_args_ammount ||(!second_arg_is_convertible_to_int)) {
    this->invalid_syntax = true;
    return;
  }
  this->job = SHELL_INSTANCE.getJobById(this->jobID);
  if (this->job == nullptr && explicit_jobID_given) {
    this->job_doesnt_exist = true;
    return;
  }
  if (this->job == nullptr && !(explicit_jobID_given) ) {
    this->job_doesnt_exist = false;
    this->jobs_empty = true;
  }

  //TODO: COMPLETE LOGIC


}

ForegroundCommand::ForegroundCommand(const argv& args, const char* cmd_line) : ForegroundCommand(args) {}


const char *ForegroundCommand::INVALID_SYNTAX_MESSAGE = "smash error: fg: invalid arguments";

const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_1 = "smash error: fg: job-id ";

const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_2 = " does not exist";

const char *ForegroundCommand::NO_JOBS_MESSAGE = "smash error: fg: jobs list is empty";

void ForegroundCommand::print_no_job_with_id() const
{
  char buf[256];
  snprintf(buf, sizeof(buf), "%s%d%s", JOB_DOESNT_EXIST_MESSAGE_1, this->jobID, JOB_DOESNT_EXIST_MESSAGE_2);
  perror(buf);
}

void ForegroundCommand::print_invalid_args() const
{
  perror(INVALID_SYNTAX_MESSAGE);
}

void ForegroundCommand::print_job_list_is_empty() const
{
  perror(NO_JOBS_MESSAGE);
}

void ForegroundCommand::execute()
{
  if (this->invalid_syntax) {
    this->print_invalid_args();
    return;
  }
  else if (this->job_doesnt_exist) {
    this->print_no_job_with_id();
    return;
  }
  else if (this->jobs_empty) {
    this->print_job_list_is_empty();
    return;
  }
  int exit_status = SHELL_INSTANCE.waitPID(this->job->getJobPID());
  //now what?? need to print something? maybe print @exit_status????
  FOR_DEBUG_MODE(printf("'void ForegroundCommand::execute()' process exit status is %d\n", exit_status); )
}

QuitCommand::QuitCommand(argv args, const char* cmd_line)
{
  killSpecified = (args[1] == "kill");
}

void QuitCommand::execute()
{
  if (killSpecified)
  {
    SHELL_INSTANCE.getJobsList().removeFinishedJobs();
    cout << "smash: sending SIGKILL signal to " << SHELL_INSTANCE.getJobsList().numberOfJobs() << " jobs" << endl;
    SHELL_INSTANCE.getJobsList().killAllJobs();
  }
  exit(0);
}

KillCommand::KillCommand(argv args, const char* cmd_line)
{
  if (args.size() > 3)
  {
    std::cerr << "smash error: invalid arguments" << endl;
  }

  signalToSend = stoi(args[1]);
  pidToSendTo = stoi(args[2]);

  if (pidToSendTo == 0)
  {
    std::cerr << "smash error: kill: job-id <job-id> does not exist" << endl;
  }
}

void KillCommand::execute()
{
  SHELL_INSTANCE.getJobsList().sendSignalToJobById(pidToSendTo, signalToSend);
}

AliasCommand::AliasCommand(argv args, const char* cmd_line)
{
  if (args.size() == 1)
  {
    aliasList = true;
  }
  else
  {
    aliasList = false;
    aliasName = extractAlias(args);
    actualCommand = extractActualCommand(args);
  }
}

void AliasCommand::execute()
{
  if (aliasName == "" || actualCommand.empty())
  {
    std::cerr << "smash error: alias: invalid alias format" << endl;
  }
  else if (aliasList)
  {
    SHELL_INSTANCE.getAliases().printAll();
  } else if (SHELL_INSTANCE.getAliases().isReserved(aliasName))
  {
    std::cerr << "smash error: alias: <name> already exists or is a reserved command " << endl;
  }
  else if(SHELL_INSTANCE.getAliases().isSyntaxValid(aliasName)){
    std::cerr << "smash error: alias: invalid alias format" << endl;
  } else{
    SHELL_INSTANCE.getAliases().addAlias(aliasName,actualCommand);
  }
}

string extractAlias(argv args)
{
 int equal = args[1].find('=');
 if (equal = -1) // = not found
 {
  return "";
 }
 return args[1].substr(0,equal); // return the string until the =
}

string extractActualCommand(argv args)
{
  int firstQuote = args[1].find('\'');
  int secondQuote = args[1].find('\'', firstQuote + 1);
  if (firstQuote == -1 || secondQuote == -1) // ' ' not found
  {
    return "";
  }
  return args[1].substr(firstQuote + 1,secondQuote - firstQuote + 1); // return the string between the ' ' in a string form
}

UnAliasCommand::UnAliasCommand(argv args, const char* cmd_line)
{
  for (int i = 1; i < args.size(); i++)
  {
    aliasesToRemove.push_back(args[i]);
  }
  if (aliasesToRemove.size() == 0)
  {
    noArgs = true;
  } else {
  noArgs = false;
  }
}

void UnAliasCommand::execute()
{
  if (noArgs)
  {
    std::cerr << "smash error: unalias: not enough arguments" << endl;
  } else {
  for (int i = 0; i < aliasesToRemove.size(); i++)
  {
    if (SHELL_INSTANCE.getAliases().doesExist(aliasesToRemove[i]))
    {
      SHELL_INSTANCE.getAliases().removeAlias(aliasesToRemove[i]);
    } else {
      std::cerr << "smash error: unalias: " << aliasesToRemove[i] << " alias does not exist" << endl;
      break;
    }
  }
  }
}

UnSetEnvCommand::UnSetEnvCommand(argv args, const char* cmd_line)
{
  variablesToRemove = extractVariables(args);
}

void UnSetEnvCommand::execute()
{
  if (variablesToRemove.size() == 0)
  {
    cerr << "smash error: unsetenv: not enough arguments";
  } else {
    for (const auto& var : variablesToRemove)
    {
      if (!(removeVariable(var)))
      {
        cerr << "smash error: unsetenv: " << var << " does not exist";
        break;
      }
    }
  }
}

argv& UnSetEnvCommand::extractVariables(argv args)
{
  argv varsToRemove;
  for (int i = 1; i < args.size(); i++)
  {
    {
      varsToRemove.push_back(args[i]);

    }
    return varsToRemove;
  }
}

bool UnSetEnvCommand::removeVariable(const string &var)
{
    for (char **env = __environ; *env != nullptr; ++env) {     // Loop through the environment array
        if (strncmp(*env, var.c_str(), var.length()) == 0 && (*env)[var.length()] == '=') { // Check if the current entry starts with var=
            char **cur = env;
            while (*(cur + 1) != nullptr) { //shift all following environment pointers one step left
                *cur = *(cur + 1); 
                ++cur;              
            }
            *cur = nullptr;  // update the environment array end
            return true;
        }
    }
    return false;
}

WatchProcCommand::WatchProcCommand(argv args, const char* cmd_line)
{
  if (args.size() == 2)
  {
    pid = static_cast<pid_t>(args[1].c_str(), nullptr, 10);
    argsFormat = true;
  }
  argsFormat = false;
}

void WatchProcCommand::execute()
{
  if (argsFormat)
  {
    if (doesPidExist())
    {
      cpuUsage = calculateCpuUsage();
      memoryUsage = calculateMemoryUsage();
      cout << "PID: " << pid << " | CPU Usage: " << cpuUsage << "%" << " | Memory Usage: " << memoryUsage << " MB" << endl;  
    } else {
      cerr << "smash error: watchproc: pid " << pid << " does not exist"; 
    }
  } else {
    cerr << "smash error: watchproc: invalid arguments";
  }
}

bool WatchProcCommand::doesPidExist()
{
    if (kill(pid, 0) == 0) {
        return true;  // process exists and have a premission to send signal
    }
    if (errno == ESRCH) { //process doesnt exist
        return false;
    }
    return true; //process exists but doesnt have a premission to send signal
}

float WatchProcCommand::calculateCpuUsage()
{
  //Read process CPU time (utime + stime)
    string path = "/proc/" + std::to_string(pid) + "/stat";

    // Open /proc/<pid>/stat for reading
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) { 
        return -1; //failed to open
    }

    char buffer1[4096] = {0}; //initalize a char array
    ssize_t bytesRead = read(fd, buffer1, sizeof(buffer1) - 1);
    close(fd);
    if (bytesRead <= 0) { 
        return -1; //failed to read
    }

    
    char* ptr = strchr(buffer1, ')'); // Skip to after the process name which is inside ()
    if (!ptr) { 
        return -1; //failed to find one of the ()
    }
    ++ptr; // Move past the closing )

    
    long utime = 0, stime = 0;
    int field = 1;
    char* token = strtok(ptr, " "); //divides the string acording to the space 
    while (token && field <= 15) { // Extract the 14th and 15th fields: utime and stime
        if (field == 13){ // field 14
        utime = atol(token);
        }  
        if (field == 14){ // field 15
        stime = atol(token);
        }  
        token = strtok(nullptr, " "); //resume the search from the same place
        ++field; //advance to the next field
    }

    long processTime = utime + stime;

   
    fd = open("/proc/stat", O_RDONLY);  //read total system CPU time from /proc/stat
    if (fd == -1) {
        return -1;  //failed to open
    }

    char buffer2[4096] = {0}; //initalize a char array
    bytesRead = read(fd, buffer2, sizeof(buffer2) - 1);
    close(fd);
    if (bytesRead <= 0) { 
        return -1; //failed to read
    }

     
    long user, nice, system, idle, iowait, irq, softirq, steal;
    if (std::sscanf(buffer2, "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) < 8) { //failed to read
        return -1;
    }

    long totalTime = user + nice + system + idle + iowait + irq + softirq + steal; //calculate total system CPU time from /proc/stat

    if (totalTime == 0){ //in order not to divide in 0
      return -1; 
    }

    float usage = ((float)processTime / totalTime) * 100.0f; //Calculate usage percentage
    return usage;
}

float WatchProcCommand::calculateMemoryUsage()
{
    string path = "/proc/" + std::to_string(pid) + "/status";
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        return -1; //failed to open
    }

    char buffer[4096] = {0};
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    if (bytesRead <= 0) {
        return -1; //failed to read
    }

    const char* keyword = "VmRSS:";
    char* line = strstr(buffer, keyword); //Search for the line starting with "VmRSS:"
    if (!line) { 
        return -1; //line not found
    }

    //Move past everything that it is not a number
    while (*line && (*line < '0' || *line > '9')){
       ++line;
    }

    
    float mem = 0;
    sscanf(line, "%d", &mem); //Extract the value in kB

    
    return (mem / 1024.0f); //Convert to MB
}

// ########################## NOTE: BuiltInCommand code area ^ ##########################

// ########################## NOTE: ExternalCommand&&ComplexExternalCommand code area V ##########################

ExternalCommand::ExternalCommand(const char *cmd_line)
{
  strcpy(this->command, cmd_line);
  this->jobPID = getpid();
}

ExternalCommand::ExternalCommand(const argv& args,const char *cmd_line) : ExternalCommand(cmd_line)
{
  // Inhereting classes can call Ctor():ExternalCommand(){} to take care of command copying;
  assert_not_empty(this->given_args);
  this->given_args = args;
}

void ExternalCommand::printYourself()
{
  printf("%s", this->command);
}

//int ExternalCommand::getPID(){return getpid();}

/*
void ExternalCommand::execute() {
  argv args& = this->given_args;


    pid_t pid = fork();

    if (pid == -1) {
      perror("fork failed");
      return;
    }

    if (pid == 0) {
      // Child process

      // Build argv array (char* array)
      char* argv[args.size() + 1];  // +1 for the nullptr at the end
      for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = strdup(args[i].c_str());  // copy the string
      }
      argv[args.size()] = nullptr;  // exec expects null-terminated array

      // Execute the external command
      execvp(argv[0], argv);

      // If execvp returns, it must have failed
      perror("execvp failed");
      // Free memory before exiting
      for (size_t i = 0; i < args.size(); ++i) {
        free(argv[i]);
      }
      exit(1);  // important: child must exit if exec fails
    } else {
      // Parent process
      int status;
      if (_isBackgroundComamnd(this->command)) {
        if (waitpid(pid, &status, 0) == -1) {
          perror("waitpid failed");
        }
      } else {
        //TODO: add child to JobList
      }
    }
}

void ExternalCommand::executeHelper() {}
*/

void ExternalCommand::execute() {
  argv& args = this->given_args;

  pid_t pid = fork();
  this->jobPID = getpid();

  if (pid == -1) {
    perror("fork failed");
    return;
  }

  if (pid == 0) { // Child
    this->executeHelper(); // <---- DELEGATE to a helper method
    exit(1); // If executeHelper returns, it means exec failed
  } else { // Parent
    int status;
    if (!_isBackgroundComamnd(this->command)) {
      do {
        FOR_DEBUG_MODE(printf("now going to wait for child in %s:%d: 'void ExternalCommand::execute()' after forking and waiting for child\n", __FILE__, __LINE__);)
        if (waitpid(pid, &status, FOREGROUND_WAIT_MODIFIER) == -1) {
          FOR_DEBUG_MODE(std::fprintf(stderr, "%s:%d: 'void ExternalCommand::execute()' after forking and waiting for child\n", __FILE__, __LINE__);)
          std::cerr << "waitpid failed";
        }
      } while (false); // close waitpid 'if' expression like was shown in lecture for 'DO_SYS' macro
    } else {
      // TODO: add job with child to JobList
    }
  }
}

void ExternalCommand::executeHelper() {
  argv& args = this->given_args;

  // Build argv array (char* array)
  char* argv[args.size() + 1]; // +1 for the nullptr at the end
  for (size_t i = 0; i < args.size(); ++i) {
    argv[i] = strdup(args[i].c_str()); // copy the string
  }
  argv[args.size()] = nullptr; // exec expects null-terminated array

  // Execute the external command
  execvp(argv[0], argv);

  // If execvp returns, it must have failed
  std::cerr << ("execvp failed");

  // Free memory before exiting
  for (size_t i = 0; i < args.size(); ++i) {
    free(argv[i]);
  }
}

void ComplexExternalCommand::executeHelper() override {
  const char* bash_path = "/bin/bash";
  const char* bash_args[] = {"bash", "-c", this->command, nullptr};

  execv(bash_path, (char* const*)bash_args);

  FOR_DEBUG_MODE(std::fprintf(stderr, "%s:%d: 'void ComplexExternalCommand::executeHelper() override' after forking and waiting for child\n", __FILE__, __LINE__);)
  std::cerr << "execv (bash) failed";
}





// ########################## NOTE: ExternalCommand&&ComplexExternalCommand code area V ##########################

// ########################## NOTE: JobList code area V ##########################

JobsList::JobEntry::JobEntry(ExternalCommand* command, int jobID) : command(command), jobID(jobID) {}

void JobsList::JobEntry::printYourself()
{
  printf("[%d] ");
  this->command->printYourself();
}

JobsList::JobsList()
{
  // TODO:
}

JobsList::~JobsList()
{
  // TODO:
}

void JobsList::addJob(ExternalCommand *cmd, bool isStopped)
{
  Jobs &jbs = this->jobs;
  int max_curr_job_id = this->get_max_current_jobID();
  int next_id = ++max_curr_job_id;
  JobEntry toInsert = JobEntry(cmd, next_id);
  jbs.insert(std::make_pair(next_id, toInsert));
}

void JobsList::printJobsList()
{
  for (auto &pair : jobs)
  {
    //int jobId = pair.first;
    JobEntry &job = pair.second;
    job.printYourself();
    printf("\n");
  }
}

void JobsList::killAllJobs()
{
  // TODO: kill all jobs
}

void JobsList::removeFinishedJobs()
{
  // TODO: remove finished jobs
}

JobsList::JobEntry* JobsList::getJobById(int jobId)
{
  auto it = this->jobs.find(jobId);
  if (it != this->jobs.end()) {
    return &(it->second);
  }
  return nullptr;
}


void JobsList::removeJobById(int jobId)
{
  this->jobs.erase(jobId); //to decide, do we want the destructor of command to be called or not?
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
  // TODO: get last job
  return nullptr;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
  // TODO: get last stopped job
  return nullptr;
}

int JobsList::get_max_current_jobID()
{
  Jobs &jbs = this->jobs;
  if (jbs.empty())
  {
    return 0;
  }
  else
  {
    int max_key = jbs.rbegin()->first;
    return max_key;
  }
}

int JobsList::numberOfJobs()
{
    return 0; //TODO
}

pid_t JobsList::JobEntry::getJobPID()
{
  return this->command->getPID();
}

void JobsList::sendSignalToJobById(int pidToSendTo, int signalToSend)
{
  //TODO
}

pid_t JobsList::getJobPID(int jobID)
{
  auto it = this->jobs.find(jobID); // search for the key
  if (it != jobs.end())
  { // found the key
    return it->second.getJobPID();
  }
  else
  {            // did not find the key
    return -1; // job with jobID does not exist currently
  }
}

// ########################## NOTE: JobList code area ^ ##########################

// ########################## NOTE: AliasHandling code area V ##########################

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

void AliasManager::removeAlias(const string& aliasToRemove){
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
bool AliasManager::doesExist(const string& newAliasName) const
{
  if (aliases.find(newAliasName) != nullptr)
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
    cout <<  alias.first << "=" << "'" << alias.second << "'" << endl;
  }
}

argv AliasManager::uncoverAlias(argv original){
  argv uncoveredArgs;

  if (original[0] == "unalias" || original[0] == "alias")
  {
      return original;
  }
  for (int i = 0; i < original.size(); i++)
  {
    if (aliases.find(original[i]) != nullptr)
    {
      istringstream iss(aliases[original[i]]);
      string word;
      while (iss >> word) {
        uncoveredArgs.push_back(word);
    }
    } else {
      uncoveredArgs.push_back(original[i]);
    }
    return uncoveredArgs;
  }
}


// ########################## NOTE: AliasHandling code area ^ ##########################
