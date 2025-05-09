//
// Created by Guy Friedman on 20/04/2025.
//

#include "BuiltInCommands.h"

#include <signal.h>

#include "SmallShellHeaders.h"

#include <unistd.h>


#include "SmallShell.h"
extern char** __environ;

ChangePromptCommand::ChangePromptCommand(const argv &args) : nextPrompt((args.size() == 1) ? SmallShell::getDefaultPrompt() : string(args[1]))
{
  assert_not_empty(args);
}

ChangePromptCommand::ChangePromptCommand(const argv &args, const char *cmd_line, const char *unused_in_builtin) : ChangePromptCommand(args) {}

void ChangePromptCommand::execute()
{
  SHELL_INSTANCE.changePrompt(this->nextPrompt);
}

ShowPidCommand::ShowPidCommand() : smashPID(SHELL_INSTANCE.getPID()) {}

ShowPidCommand::ShowPidCommand(const argv &args) : ShowPidCommand()
{
  assert_not_empty(args);
}

const char *ChangeDirCommand::TOO_MANY_ARGS = "smash error: cd: too many arguments";
const char *ChangeDirCommand::OLD_PWD_NOT_SET = "smash error: cd: OLDPWD not set";

ShowPidCommand::ShowPidCommand(const argv &args, const char *cmd_line, const char *unused_in_builtin)
    : ShowPidCommand(args) {}

void ShowPidCommand::execute()
{
  printf("smash pid is %d \n", this->smashPID);
}

GetCurrDirCommand::GetCurrDirCommand() // TODO: need to initialise fields? maybe.
{
  SHELL_INSTANCE.tryLoadShellPath(this->current_path, sizeof(this->current_path));
}

GetCurrDirCommand::GetCurrDirCommand(const argv &args) : GetCurrDirCommand()
{
  assert_not_empty(args);
}

GetCurrDirCommand::GetCurrDirCommand(const argv &args, const char *cmd_line, const char *unused_in_builtin) : GetCurrDirCommand(args) {}

void GetCurrDirCommand::execute()
{
  FOR_DEBUG_MODE(printf("file: %s , line: %d, function: void GetCurrDirCommand::execute() %s\n",__FILE__, __LINE__, __FUNCTION__);)
  printf("%s\n", this->current_path);
}

ChangeDirCommand::ChangeDirCommand(const argv &args)
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
  int given_args = args.size() - 1; // first arg is the call to the function itself, so its accounted for with the -1.
  if (given_args == 0)
  {
    this->DoNothing = true;
  }
  if (given_args > 1)
  {
    this->TooManyArgs = true;
  }

  string next_path_arg = args[1];

  if (STRINGS_EQUAL(args[1], "-"))
  {
    if (SHELL_INSTANCE.hasOldPath())
    {
      next_path_arg = SHELL_INSTANCE.getPreviousPath();
    }
    else
    {
      this->OldPWDNotSet = true;
    }
  }
  bool command_invalid = (this->DoNothing || this->TooManyArgs || this->OldPWDNotSet);
  if (!command_invalid)
  {
    strcpy(this->next_path, next_path_arg.c_str());
  }
}


ChangeDirCommand::ChangeDirCommand(const argv &args, const char *cmd_line, const char *unused_in_builtin)
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
  { // tried to load a non existing old path
    perror(this->OLD_PWD_NOT_SET);
    return;
  }
  bool succses = SHELL_INSTANCE.changeShellDirectory(this->next_path);
  if (succses)
  { // print updated path
    SHELL_INSTANCE.print_current_path();
  }
  else
  {
    FOR_DEBUG_MODE(perror("some trouble in 'void ChangeDirCommand::execute()'");)
  }
}

JobsCommand::JobsCommand(const argv &args)
{

}

JobsCommand::JobsCommand(const argv &args, const char *cmd_line, const char *unused_in_builtin)
    : JobsCommand(args)
{
  //SHELL_INSTANCE.getJobsList().removeFinishedJobs();
  }

void JobsCommand::execute()
{
  //SHELL_INSTANCE.getJobsList().removeFinishedJobs();
  if (SHELL_INSTANCE.getJobsList().numberOfJobs() == 0)
  {
    cerr << "smash error: jobs: jobs list is empty" << endl;
  } else {
    SHELL_INSTANCE.getJobsList().printJobsListWithID();
  }
}

const char *ForegroundCommand::INVALID_SYNTAX_MESSAGE = "smash error: fg: invalid arguments";
const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_1 = "smash error: fg: job-id ";
const char *ForegroundCommand::JOB_DOESNT_EXIST_MESSAGE_2 = " does not exist";
const char *ForegroundCommand::NO_JOBS_MESSAGE = "smash error: fg: jobs list is empty";

ForegroundCommand::ForegroundCommand(const argv &args)
{
  assert_not_empty(args);
  bool incorrect_args_ammount = args.size() > 2; // first arg should be "fg" and second (optional) arg should be a specific job ID
  bool explicit_jobID_given = args.size() == 2;
  bool second_arg_is_convertible_to_int;

  if (explicit_jobID_given)
  {
    second_arg_is_convertible_to_int = stringToInt(args[1], this->jobID); // if second arg was given, make sure that its an int
  }
  else
  {
    this->jobID = SHELL_INSTANCE.get_max_current_jobID();
    second_arg_is_convertible_to_int = true;
  }

  if (incorrect_args_ammount || (!second_arg_is_convertible_to_int) || jobID < 0)
  {
    this->invalid_syntax = true;
    return;
  }

  this->job = SHELL_INSTANCE.getJobById(this->jobID);
  if (this->job == nullptr && explicit_jobID_given)
  {
    this->job_doesnt_exist = true;
    return;
  }
  if (this->job == nullptr && !(explicit_jobID_given))
  {
    this->job_doesnt_exist = false;
    this->jobs_empty = true;
  }

  // TODO: COMPLETE LOGIC
}

ForegroundCommand::ForegroundCommand(const argv &args, const char *cmd_line, const char *unused_in_builtin) : ForegroundCommand(args) {}



void ForegroundCommand::print_no_job_with_id() const
{
  cerr << JOB_DOESNT_EXIST_MESSAGE_1 << this->jobID << JOB_DOESNT_EXIST_MESSAGE_2 << endl;
}

void ForegroundCommand::print_invalid_args() const
{
  cerr << INVALID_SYNTAX_MESSAGE << endl;
}

void ForegroundCommand::print_job_list_is_empty() const
{
  cerr << NO_JOBS_MESSAGE << endl;
}

void ForegroundCommand::execute()
{
  if (this->invalid_syntax)
  {
    this->print_invalid_args();
    return;
  }
  else if (this->job_doesnt_exist)
  {
    this->print_no_job_with_id();
    return;
  }
  else if (this->jobs_empty)
  {
    this->print_job_list_is_empty();
    return;
  }
  this->job->printYourselfWithPID(true);
  cout.flush();
  SHELL_INSTANCE.getJobsList().removeJobById(this->jobID);
  int exit_status = SHELL_INSTANCE.waitPID(this->job->getJobPID());
  // now what?? need to print something? maybe print @exit_status????
  FOR_DEBUG_MODE(printf("'void ForegroundCommand::execute()' process exit status is %d\n", exit_status);)
}


const char* QuitCommand::SENDING_SIGKILL = "smash: sending SIGKILL signal to ";


QuitCommand::QuitCommand(const argv& args, const char *cmd_line, const char *unused_in_builtin)
{
  if (args.size() >= 2)
  {
    killSpecified = (args[1] == "kill");
  }
  else
  {
    killSpecified = false;
  }
}

void QuitCommand::execute()
{
  if (killSpecified)
  {
    SHELL_INSTANCE.getJobsList().removeFinishedJobs();
    cout << this->SENDING_SIGKILL << SHELL_INSTANCE.getJobsList().numberOfJobs() << " jobs:" << endl;
    SHELL_INSTANCE.getJobsList().printJobsListWithPID();
    SHELL_INSTANCE.getJobsList().killAllJobs();
  }
  exit(0);
}

const char* KillCommand::INVALID_ARGUMENTS = "smash error: kill: invalid arguments";
const char* KillCommand::JOB_DOESNT_EXIST_1 = "smash error: kill: job-id ";
const char* KillCommand::JOB_DOESNT_EXIST_2 = " does not exist";

KillCommand::KillCommand(const argv& args, const char *cmd_line, const char *unused_in_builtin)
{
  numOfArgs = args.size();
  string temp = args[1];
  temp.erase(0, 1);
  signalToSend = stoi(temp);
  idToSendTo = stoi(args[2]);
  job = SHELL_INSTANCE.getJobById(idToSendTo);
}

void KillCommand::execute()
{
  if (numOfArgs > 3)
  {
    cerr << this->INVALID_ARGUMENTS << endl;
    return;
  }
  if (idToSendTo == 0 || job == nullptr )
  {
    cerr << this->JOB_DOESNT_EXIST_1 << idToSendTo << this->JOB_DOESNT_EXIST_2 << endl;
    return;
  }
  SHELL_INSTANCE.getJobsList().sendSignalToJobById(idToSendTo, signalToSend);
}

const char* AliasCommand::INVALID_FORMAT = "smash error: alias: invalid alias format";
const char* AliasCommand::ALIAS_EXISTS_1 = "smash error: alias: ";
const char* AliasCommand::ALIAS_EXISTS_2 = " already exists or is a reserved command";

AliasCommand::AliasCommand(const argv& args, const char *cmd_line, const char *unused_in_builtin)
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
  if (aliasList)
  {
    SHELL_INSTANCE.getAliases().printAll();
  }
  else if (aliasName == "" || actualCommand.empty())
  {
    cerr << this->INVALID_FORMAT << endl; 
  }
  else if (SHELL_INSTANCE.getAliases().isReserved(aliasName))
  {
    cerr << this->ALIAS_EXISTS_1 << aliasName << this->ALIAS_EXISTS_2 << endl;
  }
  else if (SHELL_INSTANCE.getAliases().isSyntaxValid(aliasName))
  {
    cerr << this->INVALID_FORMAT << endl;
  }
  else
  {
    SHELL_INSTANCE.getAliases().addAlias(aliasName, actualCommand);
  }
}

string AliasCommand::extractAlias(const argv& args)
{
  int equal = args[1].find('=');
  if (equal == -1) 
  {
    return "";
  }
  return args[1].substr(0, equal); 
}

string AliasCommand::extractActualCommand(const argv& args)
{
    string full = uniteArgs(args, 1);
    int firstQuote = full.find('\'');
    int secondQuote = full.find('\'', firstQuote + 1);
    if (firstQuote == -1 || secondQuote == -1) {
        return "";
    }

    return full.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

string AliasCommand::uniteArgs(const argv& args, int start)
{
    string full;
    for (int i = start; i < args.size(); ++i) {
        full += args[i];
        if (i != args.size() - 1) {
            full += " ";
        }
    }
    return full;
}

const char* UnAliasCommand::NOT_ENOUGH_ARGUMENTS = "smash error: unalias: not enough arguments";
const char* UnAliasCommand::ALIAS_DOESNT_EXIST_1 = "smash error: unalias: ";
const char* UnAliasCommand::ALIAS_DOESNT_EXIST_2 = "does not exist";

UnAliasCommand::UnAliasCommand(const argv& args, const char *cmd_line, const char *unused_in_builtin)
{
  for (int i = 1; i < args.size(); i++)
  {
    aliasesToRemove.push_back(args[i]);
  }
  if (aliasesToRemove.size() == 0)
  {
    noArgs = true;
  }
  else
  {
    noArgs = false;
  }
}

void UnAliasCommand::execute()
{
  if (noArgs)
  {
    cerr << this->NOT_ENOUGH_ARGUMENTS;
  }
  else
  {
    for (int i = 0; i < aliasesToRemove.size(); i++)
    {
      if (SHELL_INSTANCE.getAliases().doesExist(aliasesToRemove[i]))
      {
        SHELL_INSTANCE.getAliases().removeAlias(aliasesToRemove[i]);
      }
      else
      {
        cerr << this->ALIAS_DOESNT_EXIST_1 << aliasesToRemove[i] << this->ALIAS_DOESNT_EXIST_2 << endl;
        break;
      }
    }
  }
}

const char* UnSetEnvCommand::NOT_ENOUGH_ARGUMENTS = "smash error: unsetenv: not enough arguments";
const char* UnSetEnvCommand::VARIABLE_DOESNT_EXIST_1 = "smash error: unsetenv: ";
const char* UnSetEnvCommand::VARIABLE_DOESNT_EXIST_2 = "does not exist";

UnSetEnvCommand::UnSetEnvCommand(const argv& args, const char *cmd_line, const char *unused_in_builtin)
{
  variablesToRemove = extractVariables(args);
}

void UnSetEnvCommand::execute()
{
  if (variablesToRemove.size() == 0)
  {
    cerr << this->NOT_ENOUGH_ARGUMENTS;
  }
  else
  {
    for (const auto &var : variablesToRemove)
    {
      if (!(doesVariableExist(var)))
      {
        cerr << this->VARIABLE_DOESNT_EXIST_1 << var << this->VARIABLE_DOESNT_EXIST_2;
        break;
      }
      else {
        removeVariable(var);
      }
    }
  }
}
argv UnSetEnvCommand::extractVariables(const argv& args)
{
  argv varsToRemove;
  for (int i = 1; i < args.size(); i++)
  {
    varsToRemove.push_back(args[i]);
  }
  return varsToRemove;
}

void UnSetEnvCommand::removeVariable(const string &var)
{
  for (char** current = __environ; *current != nullptr; ++current)
  { 
    if (foundEnvVar(*current, var))
    { 
      char** nextEnv = current + 1;
            char** targetSlot = current;

            while (*nextEnv != nullptr)
            {
                *targetSlot = *nextEnv;
                ++nextEnv;
                ++targetSlot;
            }

            *targetSlot = nullptr;
            break;
    }
  }
}

bool UnSetEnvCommand::foundEnvVar(const char* entry, const string& var) {
    int nameLen = var.length();
    return strncmp(entry, var.c_str(), nameLen) == 0 && entry[nameLen] == '=';
}

bool UnSetEnvCommand::doesVariableExist(const string &var)
{
    int fd;
    TRY_SYS3(fd, open("/proc/self/environ", O_RDONLY), "open");
    if (fd == -1)
        return false;

    char buffer[4096] = {0};
    ssize_t bytesRead;
    TRY_SYS3(bytesRead, read(fd, buffer, sizeof(buffer) - 1), "read");
    TRY_SYS3(fd,close(fd),"close");

    if (bytesRead <= 0)
        return false; 

    const char* ptr = buffer;
    while (ptr < buffer + bytesRead) {
        size_t len = strlen(ptr);
        if (strncmp(ptr, var.c_str(), var.length()) == 0 && ptr[var.length()] == '=') {
            return true;
        }
        ptr += len + 1; 
    }
    return false;
}

const char* WatchProcCommand::INVALID_ARGUMENTS = "smash error: watchproc: invalid arguments";
const char* WatchProcCommand::PID_DOESNT_EXIST_1 = "smash error: watchproc: pid ";
const char* WatchProcCommand::PID_DOESNT_EXIST_2 = "does not exist";

WatchProcCommand::WatchProcCommand(const argv& args, const char *cmd_line, const char *unused_in_builtin)
{
  if (args.size() == 2)
  {
    pid = static_cast<pid_t>(stoi(args[1]));
    argsFormat = true;
  } else {
    argsFormat = false;
  }
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
    }
    else
    {
      cerr << this->PID_DOESNT_EXIST_1 << pid << " " << this->PID_DOESNT_EXIST_2 << endl;
    }
  }
  else
  {
    cerr << this->INVALID_ARGUMENTS << endl;
  }
}

bool WatchProcCommand::doesPidExist()
{
  if (kill(pid, 0) == 0)
  {
    return true; 
  }
  if (errno == ESRCH)
  { 
    return false;
  }
  return true;
}

float WatchProcCommand::calculateCpuUsage()
{
    long processTime1 = readProcessTime(pid);
    long systemTime1 = readSystemTime();

    sleep(1); 

    long processTime2 = readProcessTime(pid);
    long systemTime2 = readSystemTime();

    if (processTime1 < 0 || systemTime1 < 0 ||
        processTime2 < 0 || systemTime2 < 0)
    {
        return -1; 
    }

    long deltaProcess = processTime2 - processTime1;
    long deltaSystem = systemTime2 - systemTime1;

    if (deltaSystem == 0) return 0.0f;

    float usage = (static_cast<float>(deltaProcess) / deltaSystem) * 100.0f;
    return usage;
}

float WatchProcCommand::calculateMemoryUsage()
{
    string path = "/proc/" + std::to_string(pid) + "/status";
    int fd; 
    TRY_SYS3(fd,open(path.c_str(), O_RDONLY), "open");
    if (fd == -1)
        return -1;

    char buffer[4096] = {0};
    ssize_t bytesRead;
    TRY_SYS3(bytesRead, read(fd, buffer, sizeof(buffer) - 1), "read");
    TRY_SYS3(fd,close(fd),"close");
    if (bytesRead <= 0)
        return -1;

    istringstream iss(buffer);
    string line;
    while (getline(iss, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            istringstream linestream(line);
            string label;
            float memKb;
            linestream >> label >> memKb;
            return memKb / 1024.0f;
        }
    }
    return -1;
}

float WatchProcCommand::systemTotalTime(const char* buffer)
{
    long userTime = 0, lowPriorityTime = 0, kernelTime = 0, idleTime = 0, 
      iowaitTime = 0, hardwareInterruptTime = 0, softwareInterruptTime = 0, stolenTime = 0;
    string cpuLabel;
    istringstream iss(buffer);
    iss >> cpuLabel >> userTime >> lowPriorityTime >> kernelTime
        >> idleTime >> iowaitTime >> hardwareInterruptTime
        >> softwareInterruptTime >> stolenTime;

    if (iss.fail() || cpuLabel.substr(0,3) != "cpu") {
        return 0;
    }

    return userTime + lowPriorityTime + kernelTime + idleTime +
      iowaitTime + hardwareInterruptTime + softwareInterruptTime + stolenTime;
}

float WatchProcCommand::processTotalTime(const char* buffer)
{
    string field;
    int fieldNumber = 1;
    long utime = 0, stime = 0;
    istringstream iss(buffer);
    while (iss >> field) {
        if (field.find(')') != string::npos)
            break;
    }

    while (iss >> field) {
        if (fieldNumber == 13)
            utime = stol(field);
        else if (fieldNumber == 14)
            stime = stol(field);
        ++fieldNumber;
    }

    return utime + stime;
}

long WatchProcCommand::readProcessTime(pid_t pid) {
    string path = "/proc/" + std::to_string(pid) + "/stat";
    char buffer[4096] = {0};
    int fd;
    TRY_SYS3(fd,open(path.c_str(), O_RDONLY), "open");
    if (fd < 0) { 
      return -1;
    }
    ssize_t bytesRead;
    TRY_SYS3(bytesRead, read(fd, buffer, sizeof(buffer) - 1), "read");
    TRY_SYS3(fd,close(fd),"close");
    if (bytesRead <= 0) {
       return -1;
    }
    return processTotalTime(buffer);
}

long WatchProcCommand::readSystemTime() {
    char buffer[4096] = {0};

    int fd;
    TRY_SYS3(fd, open("/proc/stat", O_RDONLY), "open");
    ssize_t bytesRead;
    TRY_SYS3(bytesRead, read(fd, buffer, sizeof(buffer) - 1),"read");
    TRY_SYS3(fd,close(fd),"close");
    if (bytesRead <= 0) {
       return -1;
    }
    return systemTotalTime(buffer);
}
