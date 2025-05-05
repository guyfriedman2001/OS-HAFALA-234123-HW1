//
// Created by Guy Friedman on 20/04/2025.
//

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

#include "SmallShell.h"
#include "SmallShellHeaders.h"
#include "ExternalCommands.h"
#include "JobList.h"
#include "CommandFactories.h"

SmallShell& SHELL_INSTANCE = SmallShell::getInstance();

const char* SmallShell::SIGKILL_STRING_MESSAGE_1 = "smash: process ";
const char* SmallShell::SIGKILL_STRING_MESSAGE_2 = " was killed";



/*void ctrlCHandler(int sig_num) {
  printf("smash: got ctrl-C\n");
  pid_t foreground_task =  SmallShell::getInstance().get_foreground_pid();
  if (foreground_task == ERR_ARG) {
    return;
  }
  TRY_SYS(kill(foreground_task, SIGKILL), "smash error: kill failed");
  printf("%s%d%s", SmallShell::SIGKILL_STRING_MESSAGE_1, foreground_task, SmallShell::SIGKILL_STRING_MESSAGE_2);
}*/

#define RUN_DIS_MAIN false
#if RUN_DIS_MAIN
int main(int argc, char *argv[]) {
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }


    SmallShell &smash = SmallShell::getInstance();
    while (true) { //FIXME: check if its ok to use fstream here or if we need to use C utils for this
        std::cout << smash.getPrompt() << smash.getEndStr();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
        std::cout << std::endl;
    }
    return 0;
}
#endif

#define background_flag_sign_blyat ('&')

void remove_background_flag_from_da_argv_blyat(argv& args) {
  assert_not_empty(args);
  std::string& last = args.back();
  if (!last.empty() && last.back() == background_flag_sign_blyat) {
    last.pop_back();
    if (last.empty()) {
      args.pop_back();
    }
  }
}

SmallShell::SmallShell()
    : currentPrompt("smash"), promptEndChar(">"), old_path_set(false), foreground_pid(-1)
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

//inline void SmallShell::print_current_path() const
void SmallShell::print_current_path() const
{
  GetCurrDirCommand temp;
  temp.execute();
}

JobsList &SmallShell::getJobsList()
{
  return this->jobs;
}

AliasManager &SmallShell::getAliases()
{
    return this->aliases;
}

argv SmallShell::uncoverAliases(const argv &original)
{
  return aliases.uncoverAlias(original);
}

void SmallShell::print_jobs()
{
  this->jobs.printJobsListWithID();
}

JobsList::JobEntry *SmallShell::getJobById(int jobId)
{
  return this->jobs.getJobById(jobId);
}

pid_t SmallShell::get_foreground_pid()
{
  return this->foreground_pid;
}

int SmallShell::kill_foreground_process(int sig_num)
{
  pid_t foreground_pid = this->get_foreground_pid();
  if (foreground_pid == SYSTEM_CALL_ERROR) { //meaning no foreground process atm and signall should be ignored
    return 0;
  }
  return kill(foreground_pid, sig_num);
}

int SmallShell::kill_process(pid_t pid, int sig_num)
{
  //TODO
}

void SmallShell::printPrompt()
{
  printf("%s%s", this->getPrompt().c_str(), this->getEndStr().c_str());
}

bool isRedirectionCommand(const char *cmd_line)
{
  if (isIORedirectionCommand(cmd_line))
  {
    return true;
  }
  if (isPipeCommand(cmd_line))
  { //TODO: maybe we want to handle the pipe logic on a different scope? YES.
    return true;
  }
  return false;
}

/*
1. dup(int oldfd)
Duplicates oldfd into the lowest available file descriptor number.

Returns the new file descriptor (also an int).

Both FDs refer to the same underlying open file description (same file, same offset, etc.).
*/



open_flag getFlagSingleArg(const std::string& arg) {
  if (STRINGS_EQUAL(arg, "<")) {
    return O_RDONLY;
  } else if (STRINGS_EQUAL(arg, ">")) {
    return O_WRONLY | O_CREAT | O_TRUNC;
  } else if (STRINGS_EQUAL(arg, ">>")) {
    return O_WRONLY | O_CREAT | O_APPEND;
  } else if (STRINGS_EQUAL(arg, "<<")) {
    return O_RDONLY;
  } else { //wrong argument given
    return ERR_ARG;
  }
}

open_flag getFlagVectorArg(const argv& args) {
  open_flag flag = ERR_ARG;
  for (const auto& arg : args) {
    if(getFlagSingleArg(arg) != ERR_ARG) {
      flag = getFlagSingleArg(arg);
      break;
    }
  }
  return flag;
}

bool is_stderr_pipe(const argv& args)
{
  assert_not_empty(args);
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "|" || args[i] == "|&") {
      return (args[i] == "|&");
    }
  }
}

int get_arg_split_idx(const argv& args, const string& compare_blyat) {
  // Find the pipe symbol
  int pipe_index = -1;
  for (int i = 0; i < args.size(); ++i) {
    if (STRINGS_EQUAL(args[i],compare_blyat)){
      pipe_index = i;
      break;
    }
  }
  return pipe_index;
}

void split_args_by_index(const argv& args, argv& left_args, argv& right_args, int split_idx)
{
  assert(split_idx != -1); // must have | or |&
  // Split left and right commands
  left_args = argv(args.begin(), args.begin() + split_idx);
  right_args = argv(args.begin() + split_idx + 1, args.end());
}

void split_output(const argv& args, argv& left_args, argv& right_args)
{
  int idx = get_arg_split_idx(args, ">");
  int idx2 = get_arg_split_idx(args, ">>");
  int actual_idx = MAX(idx,idx2);
  split_args_by_index(args, left_args, right_args, actual_idx);
}

void split_input(const argv& args, argv& left_args, argv& right_args)
{
  int idx = get_arg_split_idx(args, "<");
  int idx2 = get_arg_split_idx(args, "<<");
  int actual_idx = MAX(idx,idx2);
  split_args_by_index(args, left_args, right_args, actual_idx);
}

void split_pipe(const argv& args, argv& left_args, argv& right_args)
{
  int idx = get_arg_split_idx(args, "|");
  int idx2 = get_arg_split_idx(args, "|&");
  int actual_idx = MAX(idx,idx2);
  split_args_by_index(args, left_args, right_args, actual_idx);
}

void applyRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)
{
  assert(isRedirectionCommand(cmd_line));
  open_flag flag = getFlagVectorArg(args);
  argv left_arguments, right_arguments;
  if (isInputRedirectionCommand(cmd_line)) {
    split_input(args, left_arguments, right_arguments);
    std_in = dup(STDIN_FILE_NUM);
    close(STDIN_FILE_NUM);
    //TODO: open child procces that would feed here the 'right side' of the given command, using 'open' and stuff
  } else if (isOutputRedirectionCommand(cmd_line)) {
    split_output(args, left_arguments, right_arguments);
    std_in = dup(STDOUT_FILE_NUM);
    close(STDOUT_FILE_NUM);
    //TODO: open child procces that would eat from here the 'left side' of the given command, using 'open' and stuff
  } else if (isPipeCommand(cmd_line)) {
    split_pipe(args, left_arguments, right_arguments);
    //TODO: utilise same functionality from above
    if (is_stderr_pipe(args))
    {
      //create cerr pipe
    } else {
      //create cout pipe
    }
  } else {
    FOR_DEBUG_MODE(
    perror("unknown redirection command in 'void applyRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)'");
    )
  }
}

void undoRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)
{
  assert(isRedirectionCommand(cmd_line));
  if (isInputRedirectionCommand(cmd_line)) {
    //close(STDIN_FILE_NUM); it appears that dup2 handles this case
    dup2(std_in, STDIN_FILE_NUM);
    close(std_in);
  } else if (isOutputRedirectionCommand(cmd_line)) {
    dup2(std_out, STDOUT_FILE_NUM);
    close(std_out);
  } else if (isPipeCommand(cmd_line)) {
    //revert input back to original
    dup2(std_in, STDIN_FILE_NUM);
    close(std_in);
    if (is_stderr_pipe(args))
    {
      //revert cerr pipe back to original
      dup2(std_err, STDERR_FILE_NUM);
      close(std_err);
    } else {
      //revert cout pipe back to original
      dup2(std_out, STDOUT_FILE_NUM);
      close(std_out);
    }
  } else {
    FOR_DEBUG_MODE(
    perror("unknown redirection command in 'void undoRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)'");
    )
  }
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

  sigset_t original_mask, new_mask;
  sigemptyset(&new_mask);
  sigaddset(&new_mask, SIGINT);
  bool isRedirectionCmd = false;

  //fd_location temp1, temp2; //here FD changes would be stored and used for reversion proccess
  fd_location std_in, std_out, std_err; //here FD changes would be stored and used for reversion proccess


  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  Command *returnCommand = nullptr;

  argv args = uncoverAliases(parseCommandLine(cmd_s));

  /*
char command_no_background[COMMAND_MAX_LENGTH];
strcpy(command_no_background, cmd_line);
_removeBackgroundSign(command_no_background);
argv args = parseCommandLine(string(command_no_background));
 */

  // char *args_[COMMAND_MAX_ARGS];
  size_t num_args = args.size(); //_parseCommandLine(cmd_line, args_); //get num of arguments
  // commandDestructor(args_, num_args);

  char afterAliases[COMMAND_MAX_LENGTH];

  if (num_args == 0)
  {
    returnCommand = new EmptyCommand(args, cmd_line);
  } else {
    remove_background_flag_from_da_argv_blyat(args);
    strcpy(afterAliases,cmd_line); //TODO MAKE ALIAS FUNCTION THAT TAKES ALAIASED ARGV AND APPLIES TO CHAR* BLYAT, for now is basic strcpy for debugging
    isRedirectionCmd = isRedirectionCommand(afterAliases);
  }




  if (isRedirectionCmd)
  {
    // mask signals and apply changes to FD
    //TRY_SYS2(sigprocmask(SIG_SETMASK, nullptr, &original_mask),"sigprocmask"); //sigprocmask - save original
    //TRY_SYS2(sigprocmask(SIG_BLOCK, &new_mask, nullptr),"sigprocmask"); //sigprocmask - block SIGINT
    TRY_SYS2(sigprocmask(SIG_SETMASK, &new_mask, &original_mask),"sigprocmask");
    applyRedirection(afterAliases, args, std_in, std_out, std_err);
  }


  if (returnCommand == nullptr)
  { // try and create a BuiltInCommand command
    BuiltInCommandFactory factory;
    returnCommand = factory.makeCommand(args, cmd_line);
    // returnCommand = BuiltInCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  { // TODO: might need a bit more logic to decide if a command is just external or special.
    SpecialCommandFactory factory;
    returnCommand = factory.makeCommand(args, cmd_line);
    // returnCommand = SpecialCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    ExternalCommandFactory factory;
    returnCommand = factory.makeCommand(args, cmd_line);
    // returnCommand = ExternalCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    Error404CommandNotFound factory;
    returnCommand = factory.makeCommand(args, cmd_line);
    // returnCommand = Error404CommandNotFound::makeCommand(args, num_args, cmd_line);
  }

  if (isRedirectionCmd)
  {
    // unmask signals and revert changes to FD
    TRY_SYS2(sigprocmask(SIG_SETMASK, &original_mask, nullptr),"sigprocmask"); //sigprocmask - restore original
    undoRedirection(afterAliases, args, std_in, std_out, std_err);
  }

  return returnCommand;
}

void SmallShell::executeCommand(const char *cmd_line)
{

  Command* cmd = this->CreateCommand(cmd_line);

  cmd->execute();

}

//void SmallShell::executeCommand(Command *command){command->execute();}

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
  return getcwd(buffer_location, buffer_size); //no need for TRY_SYS, tryLoadShellPath handles failures.
}

void SmallShell::tryLoadShellPath(char *buffer_location, size_t buffer_size)
{
  if (loadShellPath(buffer_location, buffer_size) == nullptr)
  {
    perror("smash error: getcwd failed");
    //exit(1);
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
    perror("smash error: chdir failed");
    FOR_DEBUG_MODE(
      perror("failure in 'bool SmallShell::changeShellDirectory(const char *next_dir)'\n");
      cerr << "line : " << __LINE__ << "file: " << __FILE__ << endl;
      )
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
  this->update_foreground_pid(pid);
  TRY_SYS2(waitpid(pid, &status, Block_until_the_child_terminates),"waitpid");
  this->update_foreground_pid(ERR_ARG);
  return status;
}

void SmallShell::update_foreground_pid(pid_t pid)
{
  this->foreground_pid = pid;
}

int SmallShell::get_max_current_jobID()
{
  return this->jobs.get_max_current_jobID();
}

void SmallShell::addJob(ExternalCommand *cmd, bool isStopped)
{
  this->jobs.addJob(cmd, isStopped);
}

void SmallShell::addJob(const char *cmd_line, pid_t pid, bool isStopped)
{
  this->addJob(new ExternalCommand(cmd_line, pid), isStopped);
}


