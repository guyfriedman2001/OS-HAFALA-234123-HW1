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

void SmallShell::print_jobs()
{
  this->jobs.printJobsList();
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

int SmallShell::kill_process(pid_t pid, int sig_num) {

}

void SmallShell::printPrompt()
{
  printf("%s%s", this->getPrompt().c_str(), this->getEndStr().c_str());
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

  argv args = parseCommandLine(cmd_s); // FIXME: after we make a function to return argv after aliasing, add call to that function @here

  /*
char command_no_background[COMMAND_MAX_LENGTH];
strcpy(command_no_background, cmd_line);
_removeBackgroundSign(command_no_background);
argv args = parseCommandLine(string(command_no_background));
 */

  // char *args_[COMMAND_MAX_ARGS];
  size_t num_args = args.size(); //_parseCommandLine(cmd_line, args_); //get num of arguments
  // commandDestructor(args_, num_args);

  if (num_args == 0)
  {
    returnCommand = new EmptyCommand(args, cmd_line);
  } else {
  remove_background_flag_from_da_argv_blyat(args);
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

  return returnCommand;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)




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


