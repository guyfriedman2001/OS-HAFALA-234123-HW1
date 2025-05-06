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

void create_pipe(const argv& args, argv& left_args, argv& right_args,fd_location &std_in,
                  fd_location &std_out,fd_location &std_err, bool isCerrPipe) //TODO BALAT needs testing blyat
{
  pid_t first_born = fork();
  TRY_SYS2(first_born,"fork");
  if (first_born != 0){return;} //if im the daddy

  //now im working as the first born (ya'ani ben bechor), this part will use @left_args
  //start by making a pipe
  BIBE my_pipe[BIBE_SIZE];
  TRY_SYS2(pipe(my_pipe), "pipe");

  //now after i made my pipe, i want to make myself a son to read my yapping
  pid_t second_born = fork();
  TRY_SYS2(second_born,"fork");

  if (second_born != 0) //todo: decide if i want to make a 'void SmallShell::executeCommand(const argv& args)' wrapper for 'void SmallShell::executeCommand(const char *cmd_line)', or it may cause broblems? YES, YHIHE BESEDER
  { //meaning i am the firstborn

    //close unused bibe end
    TRY_SYS2(close(my_pipe[BIBE_READ]),"close");

    //use given bool to decide if i am changing my cout or cerr into the bibe
    fd_location bibe_outbut_target = ((isCerrPipe) ? STDERR_FILE_NUM : STDOUT_FILE_NUM);

    //migrate used bibe end to stdout/stderr depending on given bool
    TRY_SYS2(dup2(my_pipe[BIBE_READ],bibe_outbut_target),"dup2");

    //close redundant bibe end
    TRY_SYS2(close(my_pipe[BIBE_WRITE]),"close");

    //use existing smash logic for the rest of the process
    SHELL_INSTANCE.executeCommand(left_args);

    //after process finished no longer need for firsborn
    exit(0);

  } else { //meaning i am the second born (yaani neched), this part would take @right_args

    //close unused bibe end
    TRY_SYS2(close(my_pipe[BIBE_WRITE]),"close");

    //migrate used bibe end to std in
    TRY_SYS2(dup2(my_pipe[BIBE_READ],STDIN_FILE_NUM),"dup2");

    //closed redundant bibe end
    TRY_SYS2(close(my_pipe[BIBE_READ]),"close");

    //use existing smash logic for the rest of the process
    SHELL_INSTANCE.executeCommand(right_args);

    //after process finished no longer need for secondborn
    exit(0);
  }
}

void applyRedirection(const char *cmd_line, const argv &args, argv &remaining_args,fd_location &std_in,fd_location &std_out,fd_location &std_err)
{
  assert(isRedirectionCommand(cmd_line));
  open_flag flag = getFlagVectorArg(args);
  argv left_arguments, right_arguments;
  if (isInputRedirectionCommand(cmd_line)) {
    split_input(args, left_arguments, right_arguments);
    std_in = dup(STDIN_FILE_NUM);
    TRY_SYS2(close(STDIN_FILE_NUM),"close");
    fd_location fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(fd,"open");
    assert(fd == STDIN_FILE_NUM);
    remaining_args = left_arguments;
  } else if (isOutputRedirectionCommand(cmd_line)) {
    split_output(args, left_arguments, right_arguments);
    std_out = dup(STDOUT_FILE_NUM);
    TRY_SYS2(close(STDOUT_FILE_NUM),"close");
    fd_location fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(fd,"open");
    assert(fd == STDOUT_FILE_NUM);
    remaining_args = left_arguments;
  } else if (isPipeCommand(cmd_line)) {
    split_pipe(args, left_arguments, right_arguments);
    create_pipe(args,left_arguments,right_arguments,std_in,std_out,std_err,is_stderr_pipe(args));
  } else {
    FOR_DEBUG_MODE(
    perror("unknown redirection command in 'void applyRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)'\n");
    )
  }
}

void undoRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)
{
  assert(isRedirectionCommand(cmd_line));
  if (isInputRedirectionCommand(cmd_line)) {
    //close(STDIN_FILE_NUM); it appears that dup2 handles this case
    TRY_SYS2(dup2(std_in, STDIN_FILE_NUM),"dup2");
    TRY_SYS2(close(std_in),"close");
  } else if (isOutputRedirectionCommand(cmd_line)) {
    TRY_SYS2(dup2(std_out, STDOUT_FILE_NUM),"dup2");
    TRY_SYS2(close(std_out),"close");
#if PIPE_CHANGES_DADDYS_FD
  } else if (isPipeCommand(cmd_line)) {
    //revert input back to original
    TRY_SYS2(dup2(std_in, STDIN_FILE_NUM),"dup2");
    TRY_SYS2(close(std_in),"close");
    if (is_stderr_pipe(args))
    {
      //revert cerr pipe back to original
      TRY_SYS2(dup2(std_err, STDERR_FILE_NUM),"dup2");
      TRY_SYS2(close(std_err),"close");
    } else {
      //revert cout pipe back to original
      TRY_SYS2(dup2(std_out, STDOUT_FILE_NUM),"dup2");
      TRY_SYS2(close(std_out),"close");
    }
#endif //if PIPE_CHANGES_DADDYS_FD
  } else {
    FOR_DEBUG_MODE(
    perror("unknown redirection command in 'void undoRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)'\n");
    )
  }
}

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{

  sigset_t original_mask, new_mask;
  sigemptyset(&new_mask);
  sigaddset(&new_mask, SIGINT);
  bool isRedirectionCmd = false;

  fd_location std_in, std_out, std_err; //here FD changes would be stored and used for reversion proccess


  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  Command *returnCommand = nullptr;

  argv args = uncoverAliases(parseCommandLine(cmd_s));

  // char *args_[COMMAND_MAX_ARGS];
  size_t num_args = args.size(); //_parseCommandLine(cmd_line, args_); //get num of arguments
  // commandDestructor(args_, num_args);

  char afterAliases[COMMAND_MAX_LENGTH];

  if (num_args == 0)
  {
    return new EmptyCommand();
  }

  remove_background_flag_from_da_argv_blyat(args);
  strcpy(afterAliases,cmd_line); //TODO MAKE ALIAS FUNCTION THAT TAKES ALAIASED ARGV AND APPLIES TO CHAR* BLYAT, for now is basic strcpy for debugging
  isRedirectionCmd = isRedirectionCommand(afterAliases);


  argv remaining_args;


  if (isRedirectionCmd)
  {
    // mask signals and apply changes to FD
    TRY_SYS2(sigprocmask(SIG_SETMASK, &new_mask, &original_mask),"sigprocmask");
    applyRedirection(afterAliases, args, remaining_args, std_in, std_out, std_err);
  } else { //if it is redirection, it would take care of remaining_args, else i need to manually initialise it for all of the args.
    remaining_args = args;
  }


  if (returnCommand == nullptr)
  { // try and create a BuiltInCommand command
    BuiltInCommandFactory factory;
    returnCommand = factory.makeCommand(remaining_args, cmd_line);
    // returnCommand = BuiltInCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  { // TODO: might need a bit more logic to decide if a command is just external or special.
    SpecialCommandFactory factory;
    returnCommand = factory.makeCommand(remaining_args, cmd_line);
    // returnCommand = SpecialCommandFactory::makeCommand(args, num_args, cmd_line);
  }

  if (returnCommand == nullptr)
  {
    ExternalCommandFactory factory;
    returnCommand = factory.makeCommand(remaining_args, cmd_line);
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

  if (isRedirectionCmd)
  {
    // unmask signals and revert changes to FD
    TRY_SYS2(sigprocmask(SIG_SETMASK, &original_mask, nullptr),"sigprocmask"); //sigprocmask - restore original
    undoRedirection(afterAliases, args, std_in, std_out, std_err); //IMPORTANT: UNDO NEEDS ALL OF THE ARGS, NOT ONLY THE REMAINING ARGS!
  }

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


