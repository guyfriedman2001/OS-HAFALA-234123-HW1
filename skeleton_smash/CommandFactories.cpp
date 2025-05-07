//
// Created by Guy Friedman on 20/04/2025.
//

#include "CommandFactories.h"
#include "SpecialCommands.h"
#include "ExternalCommands.h"

Command *BuiltInCommandFactory::factoryHelper(argv args, const char *cmd_line)
{
  string &command = args[0];
  if (STRINGS_EQUAL(command, "chprompt"))
  {
    return new ChangePromptCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "showpid"))
  {
    return new ShowPidCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "pwd"))
  {
    return new GetCurrDirCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "cd"))
  {
    return new ChangeDirCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "jobs"))
  {
    return new JobsCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "fg"))
  {
    return new ForegroundCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "quit"))
  {
    return new QuitCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "kill"))
  {
    return new KillCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "alias"))
  {
    return new AliasCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "unalias"))
  {
    return new UnAliasCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "unsetenv"))
  {
    return new UnSetEnvCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "watchproc"))
  {
    return new WatchProcCommand(args, cmd_line);
  }
  else
  { // unknown command
    return nullptr;
  }
}

Command *ExternalCommandFactory::factoryHelper(argv args, const char *cmd_line)
{
  if (SmashUtil::isCompleExternalCommand(cmd_line)) {
    return new ComplexExternalCommand(args, cmd_line);
  } else {
    return new ExternalCommand(args, cmd_line);
  }
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

Command *SpecialCommandFactory::factoryHelper(argv args, const char *cmd_line)
{
  string &command = args[0];
  if (STRINGS_EQUAL(command, "du"))
  {
    PRINT_DEBUG_MODE("DiskUsageCommand");
    return new DiskUsageCommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "whoami"))
  {
    PRINT_DEBUG_MODE("whoami command");
    return new WhoAmICommand(args, cmd_line);
  }
  else if (STRINGS_EQUAL(command, "netinfo"))
  {
    PRINT_DEBUG_MODE("netinfo command");
    return new NetInfo(args, cmd_line);
  }
  else {
    return nullptr;
  }
}

Command *Error404CommandNotFound::factoryHelper(argv args, const char *cmd_line)
{
  return new CommandNotFound(args, cmd_line);
}