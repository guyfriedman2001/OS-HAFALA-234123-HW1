#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <regex>
#include "Commands.h"

class SmallShell;

#define STRINGS_EQUAL(A, B) (strcmp((A), (B)) == 0)
#define COPY_CHAR_ARR(A, B) (while (*A++ = *B++)) // inline void strcopy(char* destination, char* origin){while(*destination++ = *origin++);}

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

#include <string>
#include <set>
#include <cstdlib>  // for getenv
#include <unistd.h> // for access()

bool isBuiltInCommand(const std::string& cmd) {
  static const std::set<std::string> builtins = {
      "cd", "pwd", "jobs", "fg", "bg", "kill", "quit", "chprompt", "showpid", "set", "unset", "alias", "unalias"
  };
  return builtins.find(cmd) != builtins.end();
}

bool isExternalComamnd(const char* cmd_line) {
  //TODO: create
  return false;
}

bool isComplexCommand(const char* cmd_line) {
  return (strchr(cmd_line, '?') || strchr(cmd_line, '*'));
}

// TODO: Add your implementation for classes in Commands.h

// ########################## NOTE: AbstractCommand code area V ##########################

int Command::getPID()
{
  // TODO: get pid of the procces running this command
}

// ########################## NOTE: AbstractCommand code area V ##########################

// ########################## NOTE: CommandFactory code area V ##########################

Command *BuiltInCommandFactory::factoryHelper(char **args, int num_args, const char *cmd_line)
{
  char *command = args[0];
  if (STRINGS_EQUAL(command, "chprompt"))
  {
    return new ChangePromptCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "showpid"))
  {
    return new ShowPidCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "pwd"))
  {
    return new GetCurrDirCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "cd"))
  {
    return new ChangeDirCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "jobs"))
  {
    return new JobsCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "fg"))
  {
    return new ForegroundCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "quit"))
  {
    return new QuitCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "kill"))
  {
    return new KillCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "alias"))
  {
    return new AliasCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "unalias"))
  {
    return new UnAliasCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "unsetenv"))
  {
    return new UnSetEnvCommand(args, num_args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "watchproc"))
  {
    return new WatchProcCommand(args, num_args, cmd_line);
  }
  else
  { // unknown command
    return nullptr;
  }
}

Command *ExternalCommandFactory::factoryHelper(char **args, int num_args, const char *cmd_line)
{
  // TODO: your implementation here
  char *command = args[0];
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

Command *SpecialCommandFactory::factoryHelper(char **args, int num_args, const char *cmd_line)
{
  // TODO: your implementation here
  char *command = args[0];
}

Command *Error404CommandNotFound::factoryHelper(char **args, int num_args, const char *cmd_line)
{
  return new CommandNotFound(args, num_args, cmd_line);
}

// ########################## NOTE: CommandFactory code area ^ ##########################

// ########################## NOTE: SmallShell code area V ##########################

SmallShell::SmallShell()
    : currentPrompt("smash"), promptEndChar(">")
{
}

SmallShell::~SmallShell()
{
  // TODO: add your implementation
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
  char *args[COMMAND_MAX_ARGS];
  int num_args = _parseCommandLine(cmd_line, args);

  if (num_args == 0)
  {
    return nullptr; // TODO: maybe make 'empty command'
  }

  if (returnCommand == nullptr)
  {
    BuiltInCommandFactory factory;
    returnCommand = factory.makeCommand(args, num_args, cmd_line);
    //returnCommand = BuiltInCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  { // TODO: might need a bit more logic to decide if a command is just external or special.
    SpecialCommandFactory factory;
    returnCommand = factory.makeCommand(args, num_args, cmd_line);
    //returnCommand = SpecialCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    ExternalCommandFactory factory;
    returnCommand = factory.makeCommand(args, num_args, cmd_line);
    //returnCommand = ExternalCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    Error404CommandNotFound factory;
    returnCommand = factory.makeCommand(args, num_args, cmd_line);
    //returnCommand = Error404CommandNotFound::makeCommand(args, num_args, cmd_line);
  }

  commandDestructor(args, num_args);
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
  SHELL_INSTANCE.currentPrompt = nextPrompt;
}

int SmallShell::getPID()
{
  // TODO: get shell pid;
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
  }
}

void SmallShell::updateOldPath()
{ // store the current path @oldPWD, to be called before path changing;
  tryLoadShellPath(this->oldPWD, sizeof(this->oldPWD));
}

bool SmallShell::changeShellDirectory(const char *next_dir)
{ // return true on succes, false on failure
  updateOldPath();
  if (chdir(next_dir) != 0)
  {
    // perror("chdir failed");
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

// ########################## NOTE: SmallShell code area ^ ##########################

// ########################## NOTE: BuiltInCommand code area V ##########################

CommandNotFound::CommandNotFound(char **args)
{
  // TODO:
}

CommandNotFound::CommandNotFound(char **args, int num_args, const char *cmd_line)
{
  // TODO: Optionally use args, num_args, or cmd_line if needed
  // Example: store cmd_line to display the unknown command later
}

void CommandNotFound::execute()
{
  // TODO:
}

ChangePromptCommand::ChangePromptCommand(char **args) : nextPrompt((args[1] == NULL) ? SmallShell::getDefaultPrompt() : std::string(args[1])) {}

ChangePromptCommand::ChangePromptCommand(char **args, int num_args, const char *cmd_line) : ChangePromptCommand(args) {}

void ChangePromptCommand::execute()
{
  SHELL_INSTANCE.changePrompt(this->nextPrompt);
}

ShowPidCommand::ShowPidCommand(char **args) : smashPID(SHELL_INSTANCE.getPID()) {}

ShowPidCommand::ShowPidCommand(char **args, int num_args, const char *cmd_line)
    : ShowPidCommand(args) {}

void ShowPidCommand::execute()
{
  printf("smash pid is %d", this->smashPID);
}

GetCurrDirCommand::GetCurrDirCommand(char **args)
{
  SHELL_INSTANCE.tryLoadShellPath(this->current_path, sizeof(this->current_path));
}

GetCurrDirCommand::GetCurrDirCommand(char **args, int num_args, const char *cmd_line)
    : GetCurrDirCommand(args) {}

void GetCurrDirCommand::execute()
{
  printf(this->current_path);
}

ChangeDirCommand::ChangeDirCommand(char **args)
{
  /** TODO: get the arg, if its empty just need to update the do nothing flag,
            is its just "-" then need to load prev directory from shell to this next dir,
            if prev path not set then need to see how to notice it and set appropriate flag
            if its an addition to a relative path, need to see how to do it
            if its to many arguments, need to set the corresponding flag

            if all flags are to remain 'false' (meaning command is ok), need
            to store the next path on this->next_path
  */
}

const char* ChangeDirCommand::TOO_MANY_ARGS = "smash error: cd: too many arguments";
const char* ChangeDirCommand::OLD_PWD_NOT_SET = "smash error: cd: OLDPWD not set";

ChangeDirCommand::ChangeDirCommand(char **args, int num_args, const char *cmd_line)
    : ChangeDirCommand(args) {}

void ChangeDirCommand::execute()
{
  if (this->DoNothing)
  { // inapropriate command handling
    return;
  }
  else if (this->TooManyArgs)
  {
    perror(this->TOO_MANY_ARGS);
    return;
  }
  else if (this->OldPWDNotSet)
  {
    perror(this->OLD_PWD_NOT_SET);
    return;
  }
  bool succses = SHELL_INSTANCE.changeShellDirectory(this->next_path);
  if (succses)
  {
    printf(this->next_path);
  }
  else
  {
    perror("some trouble in 'void ChangeDirCommand::execute()'");
  }
}

JobsCommand::JobsCommand(char **args)
{
  // TODO:
}

JobsCommand::JobsCommand(char **args, int num_args, const char *cmd_line)
    : JobsCommand(args)
{
  // TODO: finish dis
}

void JobsCommand::execute()
{
  // TODO:
}

// ForegroundCommand::ForegroundCommand(char **args) {}

ForegroundCommand::ForegroundCommand(char **args, int num_args, const char *cmd_line)
{
  // TODO: finish dis
}

void ForegroundCommand::execute()
{
  // TODO:
}

QuitCommand::QuitCommand(char **args)
{
  killSpecified = (args[1] != NULL && strcmp(args[1], "kill") == 0);
}

void QuitCommand::execute()
{
  if (killSpecified)
  {
    SHELL_INSTANCE.getJobsList().removeFinishedJobs();
    printf("smash: sending SIGKILL signal to %d jobs: \n", SHELL_INSTANCE.getJobsList().numberOfJobs());
    SHELL_INSTANCE.getJobsList().killAllJobs();
  }
  exit(0);
}

KillCommand::KillCommand(char **args, int num_args, const char *cmd_line)
{
  if (num_args > 3)
  {
    perror("smash error: invalid arguments");
  }

  signalToSend = atoi(args[1]);
  pidToSendTo = atoi(args[2]);

  if (pidToSendTo == 0)
  {
    perror("smash error: kill: job-id <job-id> does not exist");
  }
}

void KillCommand::execute()
{
  SHELL_INSTANCE.getJobsList().sendSignalToJobById(pidToSendTo, signalToSend);
}

AliasCommand::AliasCommand(char **args, int num_args, const char *cmd_line)
{
  if (args[1] == NULL)
  {
    aliasList = true;
  }
  else
  {
    aliasList = false;
    aliasName = extractAlias(cmd_line);
    actualCommand = extractActualCommand(cmd_line);
  }
}

void AliasCommand::execute()
{
  if (aliasName == "" || actualCommand == NULL)
  {
    perror("smash error: alias: invalid alias format");
  }
  else if (aliasList)
  {
    SHELL_INSTANCE.getAliases().printAll();
  } else if (SHELL_INSTANCE.getAliases().isReserved(aliasName))
  {
    perror("smash error: alias: <name> already exists or is a reserved command ");
  }
  else if(SHELL_INSTANCE.getAliases().isSyntaxValid(aliasName)){
    perror("smash error: alias: invalid alias format");
  } else{
    SHELL_INSTANCE.getAliases().addAlias(aliasName,actualCommand);
  }
}

std::string extractAlias(const char *cmd_line)
{
  const char *equalExists = strchr(cmd_line, '=');
  if (!equalExists)
  {
    return ""; // = not found thus format is invalid
  }
  return std::string(cmd_line, equalExists);
}

char *extractActualCommand(const char *cmd_line)
{
  const char *commandStart = strchr(cmd_line, '\'');
  if (!commandStart)
  {
    return NULL; // ' not found thus format is invalid
  }

  const char *commandEnd = strchr(commandStart + 1, '\'');
  if (!commandEnd)
  {
    return NULL; // ' not found thus format is invalid
  }
  size_t length = commandStart - (commandEnd + 1);
  char *actualCommand = new char[length + 1];
  strncpy(actualCommand, commandStart + 1, length);
  actualCommand[length] = '\0';

  return actualCommand;
}

UnAliasCommand::UnAliasCommand(char **args)
{
  // TODO:
}

void UnAliasCommand::execute()
{
  // TODO:
}

UnSetEnvCommand::UnSetEnvCommand(char **args)
{
  // TODO:
}

void UnSetEnvCommand::execute()
{
  // TODO:
}

WatchProcCommand::WatchProcCommand(char **args)
{
  // TODO:
}

void WatchProcCommand::execute()
{
  // TODO:
}

// ########################## NOTE: BuiltInCommand code area ^ ##########################

// ########################## NOTE: JobList code area V ##########################

const char *ForegroundCommand::INVALID_SYNTAX_MESSAGE = "smash error: fg: invalid arguments";

const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_1 = "smash error: fg: job-id ";

const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_2 = " does not exist";

const char *ForegroundCommand::NO_JOBS_MESSAGE = "smash error: fg: jobs list is empty";

void JobsList::JobEntry::printYourself()
{
  // TODO:
}

JobsList::JobsList()
{
  // TODO:
}

JobsList::~JobsList()
{
  // TODO:
}

void JobsList::addJob(Command *cmd, bool isStopped)
{
  Jobs &jbs = this->jobs;
  int max_curr_job_id = this->get_max_current_jobID();
  int next_id = ++max_curr_job_id;
  // FIXME: add call to a proper JobEntry constructor;
  JobEntry toInsert;
  jbs.insert(std::make_pair(next_id, toInsert));
}

void JobsList::printJobsList()
{
  for (auto &pair : jobs)
  {
    int jobId = pair.first;
    JobEntry &job = pair.second;
    job.printYourself();
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

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  // TODO: get job by id
  return nullptr;
}

void JobsList::removeJobById(int jobId)
{
  // TODO: remove job by id
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
    int max_key = jobs.rbegin()->first;
    return max_key;
  }
}

int JobsList::JobEntry::getPID()
{
  return this->command->getPID();
}

int JobsList::getPID(int jobID)
{
  auto it = this->jobs.find(jobID); // search for the key
  if (it != jobs.end())
  { // found the key
    return it->second.getPID();
  }
  else
  {            // did not find the key
    return -1; // job with jobID does not exist currently
  }
}

// ########################## NOTE: JobList code area ^ ##########################

// ########################## NOTE: AliasHandling code area V ##########################

void AliasManager::addAlias(const std::string &newAliasName, const char *cmd_line)
{
  if (isReserved(newAliasName))
  {
    perror("smash error: alias: <name> already exists or is a reserved command");
  }

  if (isSyntaxValid(newAliasName))
  {
    perror("smash error: alias: invalid alias format");
  }

  aliases.insert(std::make_pair(newAliasName, cmd_line));
}

#define ignore_for_now true
#if !ignore_for_now
bool AliasManager::isReserved(const std::string &newAliasName) const
{
  if (aliases.find(newAliasName) != nullptr || newAliasName == "cd" || newAliasName == "pwd" || newAliasName == "chprompt" || newAliasName == "showpid" ||
      newAliasName == "jobs" || newAliasName == "fg" || newAliasName == "alias" || newAliasName == "quit" ||
      newAliasName == "kill" || newAliasName == "unalias" || newAliasName == "unsetenv" || newAliasName == "watchproc" ||
      newAliasName == "lisidr" || newAliasName == "ls") // TODO maybe add more reserved words
  {
    return false;
  }
  return true;
}
#endif

bool AliasManager::isSyntaxValid(const std::string &newAliasName) const
{
  const std::regex pattern("^alias [a-zA-Z0-9_]+='[^']*'$");
  return std::regex_match(newAliasName, pattern);
}

void AliasManager::printAll() const
{
  for (const auto &alias : aliases)
  {
    printf("%s='%s'\n", alias.first.c_str(), alias.second);
  }
}

// ########################## NOTE: AliasHandling code area ^ ##########################
