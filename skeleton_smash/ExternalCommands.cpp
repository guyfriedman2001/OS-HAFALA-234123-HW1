//
// Created by Guy Friedman on 20/04/2025.
//

#include "ExternalCommands.h"
#include "SmallShellHeaders.h"
#include "SmallShell.h"
//SmallShell &SHELL_INSTANCE = SmallShell::getInstance();


pid_t ExternalCommand::getPID()
{
  return this->jobPID;
}

ExternalCommand::ExternalCommand(const char *cmd_line)
{
  strcpy(this->command_original, cmd_line);
  this->jobPID = getpid();
}

ExternalCommand::ExternalCommand(const char *cmd_line, pid_t pid) : ExternalCommand(cmd_line)
{
  this->jobPID = pid;
}


ExternalCommand::ExternalCommand(const argv &args, const char *cmd_line) : ExternalCommand(cmd_line)
{
  // Inhereting classes can call Ctor():ExternalCommand(){} to take care of command copying;
  assert_not_empty(args);
  this->given_args = args;
}

void ExternalCommand::printYourself()
{
  cout << this->command_original;
}

void ExternalCommand::execute()
{
  pid_t pid = fork();
  TRY_SYS2(pid, "fork");
  this->jobPID = pid;

  if (pid == 0)
  {                        // Child
    this->executeHelper(); // <---- DELEGATE to a helper method
    exit(1);               // If executeHelper returns, it means exec failed
  }
  else
  { // Parent
    int status;
    if (!_isBackgroundComamnd(this->command_original)) //we need to wait for this command
    {
      TRY_SYS2(waitpid(pid, &status, wait_but_can_still_get_ctrl_c),"waitpid");
    }
    else
    {
      SHELL_INSTANCE.addJob(this);
    }
  }
}

void ExternalCommand::executeHelper()
{
  argv &args = this->given_args;

  // Build argv array (char* array)
  char *char_argv[args.size() + 1]; // +1 for the nullptr at the end
  for (size_t i = 0; i < args.size(); ++i)
  {
    char_argv[i] = strdup(args[i].c_str()); // copy the string
  }
  char_argv[args.size()] = nullptr; // exec expects null-terminated array

  MARK_FOR_DEBUGGING_PERROR

FOR_DEBUG_MODE(
for (const auto& arg : char_argv) {
cout << "in ExternalCommand::executeHelper, argument:" << arg << endl;
}
)
  // Execute the external command
  execvp(char_argv[0], char_argv);

  // If execvp returns, it must have failed
  TRY_SYS2(ERR_ARG, "execvp");

  // Free memory before exiting
  for (size_t i = 0; i < args.size(); ++i)
  {
    free(char_argv[i]);
  }
}

#if 0
inline int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY();
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)));
  for (std::string s; iss >> s; )
  {
    args[i] = (char *)malloc(s.length() + 1);
    std::memset(args[i], 0, s.length() + 1);
    std::strcpy(args[i], s.c_str());
    ++i;
  }
  args[i] = NULL;  // Only once, after the loop!
  FUNC_EXIT();
  return i;
}

#endif
void ComplexExternalCommand::executeHelper()
{
  const char *bash_path = "/bin/bash";
  char *bash_args[COMMAND_MAX_ARGS]; //+1];
  _parseCommandLine(this->command,bash_args);

  MARK_FOR_DEBUGGING_PERROR

  FOR_DEBUG_MODE(
for (const auto& arg : bash_args) {
  cout << "in ComplexExternalCommand::executeHelper, argument:" << arg << endl;
}
  )

  execv(bash_path, bash_args);

  MARK_FOR_DEBUGGING_PERROR

  for (size_t i = 0; i < this->given_args.size(); ++i)
  {
    free(bash_args[i]);
  }

  FOR_DEBUG_MODE(std::fprintf(stderr, "%S%s:%d: 'void ComplexExternalCommand::executeHelper() override' after forking and waiting for child\n","(FORDEB~UGMODE) ", __FILE__, __LINE__);)
  //same here from "void ExternalCommand::executeHelper()"
  TRY_SYS2(ERR_ARG, "execvp");
}