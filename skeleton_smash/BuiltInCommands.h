//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef BUILTINCOMMANDS_H
#define BUILTINCOMMANDS_H
#include "SmallShellHeaders.h"
#include "JobList.h"
#include "Commands.h"

#if 1 //other headers //TODO: try and find a way to remove these inclusions and move them to "SmallShellHeaders.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cassert>
 // for std::from_chars
#include <fcntl.h>  // for open()
#include <unistd.h> //for close()
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <regex>
#include "Commands.h"
#include <string>
#include <set>
#include <cstdlib> // for getenv
#endif //other headers

class BuiltInCommand : public Command {
public:
    BuiltInCommand() = default;

    BuiltInCommand(const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    BuiltInCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~BuiltInCommand() = default;
};

class ChangePromptCommand : public BuiltInCommand { //"chprompt"
private:
    std::string nextPrompt;
public:
    explicit ChangePromptCommand(const argv& args);

    //ChangePromptCommand(char **args, int num_args, const char* cmd_line);

    ChangePromptCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~ChangePromptCommand() = default;

    void execute() override;
};


class ShowPidCommand : public BuiltInCommand { //"showpid"
    private:
        int smashPID;
    public:
        ShowPidCommand();

        explicit ShowPidCommand(const argv& args);

        //ShowPidCommand(char **args, int num_args, const char* cmd_line);

        ShowPidCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);


        virtual ~ShowPidCommand() = default;

        void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand { // AKA "pwd"
private:
    char current_path[MAX_DIR_LENGTH];
public:
    GetCurrDirCommand();

    explicit GetCurrDirCommand(const argv& args);

    //GetCurrDirCommand(char **args, int num_args, const char* cmd_line);

    GetCurrDirCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~GetCurrDirCommand() = default;

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand { // AKA "cd"
private:
    const static char* TOO_MANY_ARGS;// = "smash error: cd: too many arguments";
    const static char* OLD_PWD_NOT_SET;// = "smash error: cd: OLDPWD not set";
    bool DoNothing = false;
    bool TooManyArgs = false;
    bool OldPWDNotSet = false;
    char next_path[MAX_DIR_LENGTH];
public:
    explicit ChangeDirCommand(const argv& args);

    //ChangeDirCommand(char **args, int num_args, const char* cmd_line);

    ChangeDirCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~ChangeDirCommand() = default;

    void execute() override;
};

class JobsCommand : public BuiltInCommand {  // AKA "jobs"
    // TODO: Add your data members
public:
    explicit JobsCommand(const argv& args);

    //JobsCommand(char **args, int num_args, const char* cmd_line);

    JobsCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand { // AKA "fg"
private:
    static const char* INVALID_SYNTAX_MESSAGE;
    static const char* JOB_DOESNT_EXIST_MESSAGE_1;
    static const char* JOB_DOESNT_EXIST_MESSAGE_2;
    static const char* NO_JOBS_MESSAGE;

    bool invalid_syntax;
    bool job_doesnt_exist;
    bool jobs_empty;
    

    int jobID;

    JobsList::JobEntry* job;

    inline void print_no_job_with_id() const;

    inline void print_invalid_args() const;

    inline void print_job_list_is_empty() const;

public:
    ForegroundCommand(const argv& args);

    //ForegroundCommand(char **args, int num_args, const char* cmd_line);

    ForegroundCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~ForegroundCommand() {}

    //inline void job_doesnt_exist();

    void execute() override;
};

class QuitCommand : public BuiltInCommand { // AKA "quit"
    // TODO: Add your data members
    bool killSpecified;
    const static char* SENDING_SIGKILL;
public:
    QuitCommand(char **args);

    QuitCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);


    virtual ~QuitCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand { // AKA "kill"
    // TODO: Add your data members
    int signalToSend;
    int idToSendTo;
    int numOfArgs;
    JobsList::JobEntry* job;
    const static char* INVALID_ARGUMENTS;
    const static char* JOB_DOESNT_EXIST_1;
    const static char* JOB_DOESNT_EXIST_2;
public:
    KillCommand(char **args);

    KillCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~KillCommand() {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand { // AKA "alias"

    bool aliasList;
    string actualCommand;
    string aliasName;
    const static char* INVALID_FORMAT;
    const static char* ALIAS_DOESNT_EXIST_1;
    const static char* ALIAS_DOESNT_EXIST_2;
    const static char* ALIAS_EXISTS_1;
    const static char* ALIAS_EXISTS_2;
public:
    AliasCommand(char **args);

    AliasCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~AliasCommand() {
    }

    void execute() override;
    string extractAlias(const argv& args);
    string extractActualCommand(const argv& args);
    string uniteArgs(const argv& args, int start);
};

class UnAliasCommand : public BuiltInCommand { // AKA "unalias"
    bool noArgs;
    argv aliasesToRemove;
    const static char* NOT_ENOUGH_ARGUMENTS;
    const static char* ALIAS_DOESNT_EXIST_1;
    const static char* ALIAS_DOESNT_EXIST_2;

public:
    UnAliasCommand(char **args);

    UnAliasCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand { // AKA "unsetenv"
    argv variablesToRemove;
    const static char* NOT_ENOUGH_ARGUMENTS;
    const static char* VARIABLE_DOESNT_EXIST_1;
    const static char* VARIABLE_DOESNT_EXIST_2;
    public:
        UnSetEnvCommand(char **args);

        UnSetEnvCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

        virtual ~UnSetEnvCommand() {
        }

        void execute() override;
        argv extractVariables(const argv& args);
        void removeVariable(const string& var);
        bool doesVariableExist(const string& var);
        bool foundEnvVar(const char* entry, const string& var);
};

class WatchProcCommand : public BuiltInCommand { // AKA "watchproc"
    pid_t pid;
    bool argsFormat;
    float cpuUsage;
    float memoryUsage;
    const static char* INVALID_ARGUMENTS;
    const static char* PID_DOESNT_EXIST_1;
    const static char* PID_DOESNT_EXIST_2;
public:
    WatchProcCommand(char **args);

    WatchProcCommand(const argv& args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~WatchProcCommand() {
    }

    void execute() override;
    bool doesPidExist();
    float calculateCpuUsage();
    float calculateMemoryUsage();
    float systemTotalTime(const char* buffer);
    float processTotalTime(const char* buffer);
    long readProcessTime(pid_t pid);
    long readSystemTime();

};




#endif //BUILTINCOMMANDS_H
