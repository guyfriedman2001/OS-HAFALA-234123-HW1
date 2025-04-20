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

JobsList &SmallShell::getJobsList()
{
  return this->jobs;
}

void SmallShell::print_jobs()
{
  this->jobs.printJobsList();
}

JobsList::JobEntry *SmallShell::getJobById(int jobId)
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
  argv args = argv(); // FIXME: after we make a function to return argv after aliasing, add call to that function @here

  // char *args_[COMMAND_MAX_ARGS];
  size_t num_args = args.size(); //_parseCommandLine(cmd_line, args_); //get num of arguments
  // commandDestructor(args_, num_args);

  if (num_args == 0)
  {
    returnCommand = new EmptyCommand(args, cmd_line); // TODO: maybe make 'empty command'
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

  /*
  Command* cmd = this->createCommand(cmd_line);
  cmd->execute();
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
  waitpid(pid, &status, Block_until_the_child_terminates);
  return status;
}

int SmallShell::get_max_current_jobID()
{
  return this->jobs.get_max_current_jobID();
}
