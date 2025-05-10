//
// Created by Guy Friedman on 20/04/2025.
//

#include "CommandFactories.h"
#include "SpecialCommands.h"
#include "ExternalCommands.h"

Command *BuiltInCommandFactory::factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases)
{
  if(args.empty())
  {
    return nullptr;
  }
  string &command = args[0];
  if (STRINGS_EQUAL(command, "chprompt"))
  {
    PRINT_DEBUG_MODE("ChangePromptCommand");
    return new ChangePromptCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "showpid"))
  {
    PRINT_DEBUG_MODE("ShowPidCommand");
    return new ShowPidCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "pwd"))
  {
    PRINT_DEBUG_MODE("GetCurrDirCommand");
    return new GetCurrDirCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "cd"))
  {
    PRINT_DEBUG_MODE("ChangeDirCommand");
    return new ChangeDirCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "jobs"))
  {
    PRINT_DEBUG_MODE("JobsCommand");
    return new JobsCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "fg"))
  {
    PRINT_DEBUG_MODE("ForegroundCommand");
    return new ForegroundCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "quit"))
  {
    PRINT_DEBUG_MODE("QuitCommand");
    return new QuitCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "kill"))
  {
    PRINT_DEBUG_MODE("KillCommand");
    return new KillCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "alias"))
  {
    PRINT_DEBUG_MODE("AliasCommand");
    return new AliasCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "unalias"))
  {
    PRINT_DEBUG_MODE("UnAliasCommand");
    return new UnAliasCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "unsetenv"))
  {
    PRINT_DEBUG_MODE("UnSetEnvCommand");
    return new UnSetEnvCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "watchproc"))
  {
    PRINT_DEBUG_MODE("WatchProcCommand");
    return new WatchProcCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else
  { // unknown command
    return nullptr;
  }
}

Command *ExternalCommandFactory::factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases)
{
  if (SmashUtil::isCompleExternalCommand(cmd_line_after_aliases)) {
    PRINT_DEBUG_MODE("ComplexExternalCommand");
    return new ComplexExternalCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  } else {
    PRINT_DEBUG_MODE("ExternalCommand");
    return new ExternalCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
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

Command *SpecialCommandFactory::factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases)
{
  if(args.empty())
  {
    return nullptr;
  }
  string &command = args[0];
  if (STRINGS_EQUAL(command, "du"))
  {
    PRINT_DEBUG_MODE("DiskUsageCommand");
    return new DiskUsageCommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "whoami"))
  {
    PRINT_DEBUG_MODE("whoami command");
    return new WhoAmICommand(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else if (STRINGS_EQUAL(command, "netinfo"))
  {
    PRINT_DEBUG_MODE("netinfo command");
    return new NetInfo(args, cmd_line_after_aliases, cmd_line_before_aliases);
  }
  else {
    return nullptr;
  }
}

#if 0
Command *Error404CommandNotFound::factoryHelper(argv args, const char *cmd_line)
{
  return new CommandNotFound(args, cmd_line);
}
#endif
