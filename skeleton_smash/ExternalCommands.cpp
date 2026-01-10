//
// Created by Guy Friedman on 20/04/2025.
//

#include "ExternalCommands.h"
#include "SmallShellHeaders.h"


pid_t ExternalCommand::getPID()
{
  return this->jobPID;
}

ExternalCommand::ExternalCommand(const char *cmd_line)
{
  strcpy(this->command, cmd_line);
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
  printf("%s", this->command);
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
    if (!_isBackgroundComamnd(this->command)) //we need to wait for this command
    {
      TRY_SYS2(waitpid(pid, &status, Block_until_the_child_terminates),"waitpid");
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

void ComplexExternalCommand::executeHelper() override
{
  const char *bash_path = "/bin/bash";
  char *bash_args[COMMAND_MAX_ARGS];
  _parseCommandLine(this->command,bash_args);

  execv(bash_path, bash_args);

  for (size_t i = 0; i < this->given_args.size(); ++i)
  {
    free(bash_args[i]);
  }

  FOR_DEBUG_MODE(std::fprintf(stderr, "%s:%d: 'void ComplexExternalCommand::executeHelper() override' after forking and waiting for child\n", __FILE__, __LINE__);)

  //same here from "void ExternalCommand::executeHelper()"
  TRY_SYS2(ERR_ARG, "execvp");
}