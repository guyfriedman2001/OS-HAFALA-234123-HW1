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

ExternalCommand::ExternalCommand(const argv &args, const char *cmd_line) : ExternalCommand(cmd_line)
{
  // Inhereting classes can call Ctor():ExternalCommand(){} to take care of command copying;
  assert_not_empty(this->given_args);
  this->given_args = args;
}

void ExternalCommand::printYourself()
{
  printf("%s", this->command);
}

// int ExternalCommand::getPID(){return getpid();}

/*
void ExternalCommand::execute() {
  argv args& = this->given_args;


    pid_t pid = fork();

    if (pid == -1) {
      perror("fork failed");
      return;
    }

    if (pid == 0) {
      // Child process

      // Build argv array (char* array)
      char* argv[args.size() + 1];  // +1 for the nullptr at the end
      for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = strdup(args[i].c_str());  // copy the string
      }
      argv[args.size()] = nullptr;  // exec expects null-terminated array

      // Execute the external command
      execvp(argv[0], argv);

      // If execvp returns, it must have failed
      perror("execvp failed");
      // Free memory before exiting
      for (size_t i = 0; i < args.size(); ++i) {
        free(argv[i]);
      }
      exit(1);  // important: child must exit if exec fails
    } else {
      // Parent process
      int status;
      if (_isBackgroundComamnd(this->command)) {
        if (waitpid(pid, &status, 0) == -1) {
          perror("waitpid failed");
        }
      } else {
        //TODO: add child to JobList
      }
    }
}

void ExternalCommand::executeHelper() {}
*/

void ExternalCommand::execute()
{
  argv &args = this->given_args;

  pid_t pid = fork();
  this->jobPID = pid;

  if (pid == -1)
  {
    perror("fork failed");
    return;
  }

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
      do
      {
        FOR_DEBUG_MODE(printf("now going to wait for child in %s:%d: 'void ExternalCommand::execute()' after forking and waiting for child\n", __FILE__, __LINE__);)
        if (waitpid(pid, &status, Block_until_the_child_terminates) == -1)
        {
          FOR_DEBUG_MODE(fprintf(stderr, "%s:%d: 'void ExternalCommand::execute()' after forking and waiting for child\n", __FILE__, __LINE__);)
          std::cerr << "waitpid failed";
        }
      } while (false); // close waitpid->'if' expression like was shown in lecture for 'DO_SYS' macro
    }
    else
    {
      // TODO: add job with child to JobList
    }
  }
}

void ExternalCommand::executeHelper()
{
  argv &args = this->given_args;

  // Build argv array (char* array)
  char *argv[args.size() + 1]; // +1 for the nullptr at the end
  for (size_t i = 0; i < args.size(); ++i)
  {
    argv[i] = strdup(args[i].c_str()); // copy the string
  }
  argv[args.size()] = nullptr; // exec expects null-terminated array

  // Execute the external command
  execvp(argv[0], argv);

  // If execvp returns, it must have failed
  std::cerr << ("execvp failed");

  // Free memory before exiting
  for (size_t i = 0; i < args.size(); ++i)
  {
    free(argv[i]);
  }
}

void ComplexExternalCommand::executeHelper() override
{
  const char *bash_path = "/bin/bash";
  const char *bash_args[] = {"bash", "-c", this->command, nullptr};

  execv(bash_path, (char *const *)bash_args);

  FOR_DEBUG_MODE(std::fprintf(stderr, "%s:%d: 'void ComplexExternalCommand::executeHelper() override' after forking and waiting for child\n", __FILE__, __LINE__);)
  std::cerr << "execv (bash) failed";
}