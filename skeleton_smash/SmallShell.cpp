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
const char* SmallShell::SIGKILL_STRING_MESSAGE_2 = " was killed\n";




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
    : currentPrompt("smash"), promptEndChar("> "), old_path_set(false), foreground_pid(NO_CURRENT_EXTERNAL_FOREGROUND_PROCESS_PID),
      m_fdmanager(FdManager::getFDManager())
{
}

SmallShell::~SmallShell()
{
  //undoRedirection(); //TODO: need to make sure that this is called in the destructor of fdmanager
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

void fillBufferWithArgs(char* buffer, const std::vector<std::string>& args) {
  buffer[0] = '\0';  // Start with an empty string

  for (size_t i = 0; i < args.size(); ++i) {
    std::strcat(buffer, args[i].c_str());
    if (i != args.size() - 1) {
      std::strcat(buffer, " ");
    }
  }
}


/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line_to_store)
{

#if 0 //FIXME - ADD TEMPORARY CHANGES REVOKE FOR SIGNAL HANDLERS.
  sigset_t original_mask, new_mask;
  sigemptyset(&new_mask);
  sigaddset(&new_mask, SIGINT);
#endif

  bool isRedirectionCmd = false;

  fd_location std_in, std_out, std_err; //here FD changes would be stored and used for reversion proccess


  string cmd_s = _trim(string(cmd_line_to_store));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  Command *returnCommand = nullptr;

  argv args = uncoverAliases(parseCommandLine(cmd_s));

  char afterAliases[COMMAND_MAX_LENGTH]; //
  fillBufferWithArgs(afterAliases, args);
  remove_background_flag_from_da_argv_blyat(args);

  argv remaining_args;


  m_fdmanager.applyRedirection(afterAliases, args, remaining_args);


  if (returnCommand == nullptr)
  { // try and create a BuiltInCommand command
    BuiltInCommandFactory factory;
    returnCommand = factory.makeCommand(remaining_args, afterAliases,cmd_line_to_store); //maybe change to pass by reference
    // returnCommand = BuiltInCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    SpecialCommandFactory factory;
    returnCommand = factory.makeCommand(remaining_args, afterAliases,cmd_line_to_store); //maybe change to pass by reference
    // returnCommand = SpecialCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    ExternalCommandFactory factory;
    returnCommand = factory.makeCommand(remaining_args, afterAliases,cmd_line_to_store); //maybe change to pass by reference
    // returnCommand = ExternalCommandFactory::makeCommand(args, num_args, cmd_line);
  }

#if !UNFOUND_COMMAND_HANDLED_AUTOMATICALLY
  if (returnCommand == nullptr)
  {
    Error404CommandNotFound factory;
    returnCommand = factory.makeCommand(remaining_args, cmd_line);
    // returnCommand = Error404CommandNotFound::makeCommand(args, num_args, cmd_line);
  }
#endif//if UNFOUND_COMMAND_HANDLED_AUTOMATICALLY

#if 0
  if (isRedirectionCmd)
  {
    // unmask signals and revert changes to FD
    TRY_SYS2(sigprocmask(SIG_SETMASK, &original_mask, nullptr),"sigprocmask"); //sigprocmask - restore original
    undoRedirection(afterAliases, args, std_in, std_out, std_err); //IMPORTANT: UNDO NEEDS ALL OF THE ARGS, NOT ONLY THE REMAINING ARGS!
  }
#endif

  return returnCommand;
}

void SmallShell::join_argv_to_cstr(const argv& args, char* buffer, size_t max_len) {
  std::string combined;

  for (size_t i = 0; i < args.size(); ++i) {
    combined += args[i];
    if (i + 1 < args.size()) {
      combined += ' ';
    }
  }
  // Make sure to null-terminate and not overflow
  std::strncpy(buffer, combined.c_str(), max_len - 1);
  buffer[max_len - 1] = '\0';  // ensure null termination
}

void SmallShell::executeCommand(const argv& args)
{
  char args_in_char_chochavit[COMMAND_MAX_LENGTH];
  join_argv_to_cstr(args, args_in_char_chochavit, COMMAND_MAX_LENGTH);
  this->executeCommand(args_in_char_chochavit);
}

bool isEmptyCommand(const char *cmd_line)
{
  bool option1 = (cmd_line == nullptr);
  if (option1){return true;}
  bool option2 = (cmd_line[0] == '\0');
  bool option3 = ((cmd_line[0] == '\n')&&(cmd_line[1] == '\0'));
  return option2 || option3;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  if (!isEmptyCommand(cmd_line)) {
    Command* cmd = this->CreateCommand(cmd_line);

    cmd->execute();

    m_fdmanager.undoRedirection();
  } else {
    EmptyCommand cmd;
    cmd.execute();
  }


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
  this->update_foreground_pid(NO_CURRENT_EXTERNAL_FOREGROUND_PROCESS_PID);
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

void SmallShell::addJob(const char *cmd_line_after_aliases, const char *cmd_line_before_aliases, pid_t pid, bool isStopped)
{
  this->addJob(new ExternalCommand(cmd_line_after_aliases, cmd_line_before_aliases, pid), isStopped);
}

void SmallShell::return_from_temporary_suspension_to_what_was_changed()
{
  this->m_fdmanager.return_from_temporary_suspension_to_what_was_changed();
}
void SmallShell::temporairly_suspend_redirection_and_return_to_default()
{
  this->m_fdmanager.temporairly_suspend_redirection_and_return_to_default();
}
void SmallShell::undoRedirection()
{
  this->m_fdmanager.undoRedirection();
}
void SmallShell::applyRedirection(const char *cmd_line, const argv &args, argv &remaining_args)
{
  this->m_fdmanager.applyRedirection(cmd_line, args, remaining_args);
}

bool SmallShell::has_foreground_process() const {
  return this->foreground_pid != NO_CURRENT_EXTERNAL_FOREGROUND_PROCESS_PID;
}



