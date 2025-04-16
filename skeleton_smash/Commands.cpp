#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#define STRINGS_EQUAL(A, B) (strcmp((A), (B)) == 0)
#define COPY_CHAR_ARR(A,B) (while(*A++ = *B++)) //inline void strcopy(char* destination, char* origin){while(*destination++ = *origin++);}

// Global alias for the singleton instance
SmallShell& SHELL_INSTANCE = SmallShell::getInstance();

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
inline void commandDestructor(char **args, int args_num){
  for (int i = 0; i < args_num; ++i) {
    if (args[i] != NULL) {
        free(args[i]);  // Free the memory allocated for each argument
        args[i] = NULL;  // Optional: Set the element to NULL to avoid dangling pointers
    }
  }
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












// ########################## NOTE: CommandFactory code area V ##########################

Command* BuiltInCommandFactory::factoryHelper(char **args, int num_args, const char* cmd_line) {
  char* command = args[0];
  if (STRINGS_EQUAL(command, "chprompt")) {
    return new ChangePromptCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "showpid")) {
      return new ShowPidCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "pwd")) {
      return new GetCurrDirCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "cd")) {
      return new ChangeDirCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "jobs")) {
      return new JobsCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "fg")) {
      return new ForegroundCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "quit")) {
      return new QuitCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "kill")) {
      return new KillCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "alias")) {
      return new AliasCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "unalias")) {
      return new UnAliasCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "unsetenv")) {
      return new UnSetEnvCommand(args,num_args,cmd_line);
  } else if (STRINGS_EQUAL(command, "watchproc")) {
      return new WatchProcCommand(args,num_args,cmd_line);
  } else { // unknown command
      return nullptr;
  }
}

Command* ExternalCommandFactory::factoryHelper(char **args, int num_args, const char* cmd_line) {
  // TODO: your implementation here
  char* command = args[0];

}

Command* SpecialCommandFactory::factoryHelper(char **args, int num_args, const char* cmd_line) {
  // TODO: your implementation here
  char* command = args[0];

}

Command* Error404CommandNotFound::factoryHelper(char **args, int num_args, const char* cmd_line) {
  return new CommandNotFound(args);
}

// ########################## NOTE: CommandFactory code area ^ ##########################

















// ########################## NOTE: SmallShell code area V ##########################

SmallShell::SmallShell()
    : currentPrompt("smash"), promptEndChar(">") {
}
  
SmallShell::~SmallShell() {
  // TODO: add your implementation
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

  Command* returnCommand = nullptr;
  char* args[COMMAND_MAX_ARGS];
  int num_args = _parseCommandLine(cmd_line,args);

  if (num_args == 0){
    return nullptr; //TODO: maybe make 'empty command'
  }

  if (returnCommand == nullptr){
    returnCommand = BuiltInCommandFactory::makeCommand(args);
  }

  if (returnCommand == nullptr){
    returnCommand = ExternalCommandFactory::makeCommand(args);
  }

  if (returnCommand == nullptr){ // TODO: might need a bit more logic to decide if a command is just external or special.
    returnCommand = SpecialCommandFactory::makeCommand(args);
  }

  if (returnCommand == nullptr){
    returnCommand = Error404CommandNotFound::makeCommand(args);
  }

  commandDestructor(args,num_args);
  return returnCommand;
}

void SmallShell::executeCommand(const char *cmd_line) {
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

void SmallShell::executeCommand(Command *command){

}

std::string SmallShell::getDefaultPrompt(){
  return "smash";
}

void SmallShell::changePrompt(std::string nextPrompt) {
  SHELL_INSTANCE.currentPrompt = nextPrompt;
}

int SmallShell::getPID(){
  //TODO: get shell pid;
}

char* SmallShell::loadShellPath(char* buffer_location, size_t buffer_size){
  return getcwd(buffer_location, buffer_size);
}

void SmallShell::tryLoadShellPath(char* buffer_location, size_t buffer_size){
  if (loadShellPath(buffer_location, buffer_size) == nullptr) {
    perror("smash error: getcwd failed");
  }
}

void SmallShell::updateOldPath(){ //store the current path @oldPWD, to be called before path changing;
  tryLoadShellPath(this->oldPWD, sizeof(this->oldPWD));
}

bool SmallShell::changeShellDirectory(const char* next_dir){ //return true on succes, false on failure
  updateOldPath();
  if (chdir(next_dir) != 0) {
    //perror("chdir failed");
    return false;
  } else {
    return true;
  }
}

std::string SmallShell::getPrompt(){
  return this->currentPrompt;
}

std::string SmallShell::getEndStr(){
  return this->promptEndChar;
}




// ########################## NOTE: SmallShell code area ^ ##########################












// ########################## NOTE: BuiltInCommand code area V ##########################


CommandNotFound::CommandNotFound(char **args) {
  // TODO:
}

CommandNotFound::CommandNotFound(char **args, int num_args, const char* cmd_line) {
  // TODO: Optionally use args, num_args, or cmd_line if needed
  // Example: store cmd_line to display the unknown command later
}

void CommandNotFound::execute() {
  // TODO:
}

ChangePromptCommand::ChangePromptCommand(char **args) : 
  nextPrompt((args[1] == NULL) ? SmallShell::getDefaultPrompt() : std::string(args[1])) {}

ChangePromptCommand::ChangePromptCommand(char **args, int num_args, const char* cmd_line) :
  ChangePromptCommand(args) {}


void ChangePromptCommand::execute() {
  SHELL_INSTANCE.changePrompt(this->nextPrompt);
}

ShowPidCommand::ShowPidCommand(char **args) : 
  smashPID(SHELL_INSTANCE.getPID()){}

ShowPidCommand::ShowPidCommand(char **args, int num_args, const char* cmd_line)
  : ShowPidCommand(args) {}


void ShowPidCommand::execute() {
    printf("smash pid is %d", this->smashPID);
}

GetCurrDirCommand::GetCurrDirCommand(char **args) {
  SHELL_INSTANCE.tryLoadShellPath(this->current_path, sizeof(this->current_path));
}

GetCurrDirCommand::GetCurrDirCommand(char **args, int num_args, const char* cmd_line)
    : GetCurrDirCommand(args) {}


void GetCurrDirCommand::execute() {
  printf(this->current_path);
}

ChangeDirCommand::ChangeDirCommand(char **args) {
  /** TODO: get the arg, if its empty just need to update the do nothing flag,
            is its just "-" then need to load prev directory from shell to this next dir,
            if prev path not set then need to see how to notice it and set appropriate flag
            if its an addition to a relative path, need to see how to do it
            if its to many arguments, need to set the corresponding flag

            if all flags are to remain 'false' (meaning command is ok), need
            to store the next path on this->next_path
  */
}

ChangeDirCommand::ChangeDirCommand(char **args, int num_args, const char* cmd_line)
    : ChangeDirCommand(args) {}

void ChangeDirCommand::execute() {
  if (this->DoNothing){ //inapropriate command handling
    return;
  } else if (this->TooManyArgs){
    perror(this->TOO_MANY_ARGS);
    return;
  } else if (this->OldPWDNotSet) {
    perror(this->TOO_MANY_ARGS);
    return;
  }
  bool succses = SHELL_INSTANCE.changeShellDirectory(this->next_path);
  if (succses){
    printf(this->next_path);
  } else {
    perror("some trouble in 'void ChangeDirCommand::execute()'");
  }
}

JobsCommand::JobsCommand(char **args) {
  // TODO:
}

JobsCommand::JobsCommand(char **args, int num_args, const char* cmd_line)
    : JobsCommand(args) {
      //TODO: finish dis
}

void JobsCommand::execute() {
  // TODO:
}

ForegroundCommand::ForegroundCommand(char **args) {
  // TODO:
}

void ForegroundCommand::execute() {
  // TODO:
}

QuitCommand::QuitCommand(char **args) {
  // TODO:
}

void QuitCommand::execute() {
  // TODO:
}

KillCommand::KillCommand(char **args) {
  // TODO:
}

void KillCommand::execute() {
  // TODO:
}

AliasCommand::AliasCommand(char **args) {
  // TODO:
}

void AliasCommand::execute() {
  // TODO:
}

UnAliasCommand::UnAliasCommand(char **args) {
  // TODO:
}

void UnAliasCommand::execute() {
  // TODO:
}

UnSetEnvCommand::UnSetEnvCommand(char **args) {
  // TODO:
}

void UnSetEnvCommand::execute() {
  // TODO:
}

WatchProcCommand::WatchProcCommand(char **args) {
  // TODO:
}

void WatchProcCommand::execute() {
  // TODO:
}

// ########################## NOTE: BuiltInCommand code area ^ ##########################









// ########################## NOTE: JobList code area V ##########################

void JobsList::JobEntry::printYourself(){
  //TODO:
}


JobsList::JobsList() {
    // TODO:
}

JobsList::~JobsList() {
    // TODO:
}

void JobsList::addJob(Command *cmd, bool isStopped) {
  Jobs& jbs = this->jobs;
  int max_curr_job_id = this->get_max_current_jobID();
  int next_id = ++max_curr_job_id;
  //FIXME: add call to a proper JobEntry constructor;
  JobEntry toInsert;
  jbs.insert(std::make_pair(next_id, toInsert));
}

void JobsList::printJobsList() {
  for (auto& pair : jobs) {
      int jobId = pair.first;
      JobEntry& job = pair.second;
      job.printYourself();
  }
}


void JobsList::killAllJobs() {
    // TODO: kill all jobs
}

void JobsList::removeFinishedJobs() {
    // TODO: remove finished jobs
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    // TODO: get job by id
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
    // TODO: remove job by id
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    // TODO: get last job
    return nullptr;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    // TODO: get last stopped job
    return nullptr;
}

int JobsList::get_max_current_jobID(){
    Jobs& jbs = this->jobs;
    if (jbs.empty()){
      return 0;
    } else {
      int max_key = jobs.rbegin()->first;
      return max_key;
    }
}


// ########################## NOTE: JobList code area ^ ##########################
