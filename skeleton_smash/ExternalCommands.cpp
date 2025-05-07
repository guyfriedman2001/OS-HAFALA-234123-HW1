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
  pid_t kid_pid = fork();
  TRY_SYS2(kid_pid, "fork");
  this->jobPID = kid_pid;

  bool is_kid = (kid_pid == 0);
  if (is_kid){
    // Child
    this->executeHelper(); // <---- DELEGATE to a helper method
    exit(1);               // If executeHelper returns, it means exec failed
  }else{ // Parent
    int status;
    do {
      if (!_isBackgroundComamnd(this->command_original)) //we need to wait for this command
      {
        TRY_SYS2(waitpid(kid_pid, &status, wait_but_can_still_get_ctrl_c),"waitpid");
      }
      else
      {
        SHELL_INSTANCE.addJob(this);
      }
    } while (0);
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

  #define shut_up_for_now true
  #if !shut_up_for_now
    MARK_FOR_DEBUGGING_PERROR

  FOR_DEBUG_MODE(
  for (const auto& arg : char_argv) {
  cout << "in ExternalCommand::executeHelper, argument:" << arg << endl;
  }
  )
  #endif

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

void yedideinu_azor_li(char* command, char** argv) {
  int i = 0;
  char* token = strtok(command, " \t\n");

  while (token != nullptr && i < COMMAND_MAX_ARGS) {
    argv[i++] = token;
    token = strtok(nullptr, " \t\n");
  }

  assert(i==COMMAND_MAX_ARGS);
  argv[i] = nullptr;  // Null-terminate the array
}


void ComplexExternalCommand::executeHelper()
{
  #define try_use_argv  true
  #if !try_use_argv
  const char *bash_path = "/bin/bash";
  char *bash_args[COMMAND_MAX_ARGS+1];

  #define use_give_parser false
  #if use_give_parser
  _parseCommandLine(this->command_original,bash_args);
  #else
  yedideinu_azor_li(this->command_original,bash_args);
  #endif
  execv(bash_path, bash_args);

  MARK_FOR_DEBUGGING_PERROR

  for (size_t i = 0; i < this->given_args.size(); ++i)
  {
    free(bash_args[i]);
  }

  FOR_DEBUG_MODE(std::fprintf(stderr, "%s%s:%d: 'void ComplexExternalCommand::executeHelper() override' after forking and waiting for child\n","(FORDEB~UGMODE) ", __FILE__, __LINE__);)
  //same here from "void ExternalCommand::executeHelper()"
  TRY_SYS2(ERR_ARG, "execvp");
  FOR_DEBUG_MODE(
for (const auto& arg : bash_args) {
cout << "in ComplexExternalCommand::executeHelper, argument:" << arg << endl;
}
)
  #else
  std::vector<char*> argv_char;
  std::vector<std::string> extra_args = {"/bin/bash", "-c"};
  for (const std::string& arg : extra_args) {
    argv_char.push_back(const_cast<char*>(arg.c_str()));  // execv requires char*
  }
  for (const std::string& arg : given_args) {
    argv_char.push_back(const_cast<char*>(arg.c_str()));  // execv requires char*
  }
  argv_char.push_back(nullptr); // Null-terminate the array

  execv(argv_char[0], argv_char.data());
  TRY_SYS2(ERR_ARG, "execv");
  #endif

  MARK_FOR_DEBUGGING_PERROR




}
#if 0
hdtjyfthdshjrt

shdjdh
dfjd
djf
fjfjd
j
dfjd
fjdj
decltype(jdj
  df
  jf
  jf
  jf


  clion thinks i didnt change the code enough amd is not letting me rebuild project blyaat
  )
#endif