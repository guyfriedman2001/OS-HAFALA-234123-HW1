//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef SMALLSHELL_H
#define SMALLSHELL_H
#include "SmallShellHeaders.h"
#include "JobList.h"
#include "ExternalCommands.h"
#include "BuiltInCommands.h"
#include "AliasManager.h"

class SmallShell {
private:
    // TODO: Add your data members
    string currentPrompt;
    string promptEndChar;
    char oldPWD[MAX_DIR_LENGTH];
    bool old_path_set;
    SmallShell();
    JobsList jobs;
    AliasManager aliases;
    pid_t foreground_pid;
    char* loadShellPath(char* buffer_location, size_t buffer_size);

public:
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);

    //void executeCommand(Command *command);

    static std::string getDefaultPrompt();

    void changePrompt(std::string nextPrompt);

    int getPID();

    void tryLoadShellPath(char* buffer_location, size_t buffer_size);

    void updateOldPath();

    bool hasOldPath();

    string getPreviousPath();

    bool changeShellDirectory(const char* next_dir);

    string getPrompt();

    string getEndStr();

    //inline void print_current_path() const;
    void print_current_path() const;

    JobsList& getJobsList();

    AliasManager& getAliases();

    argv uncoverAliases(const argv& original);

    void print_jobs();

    int waitPID(pid_t pid);

    JobsList::JobEntry *getJobById(int jobId);

    int get_max_current_jobID();

    pid_t get_foreground_pid();

    int kill_foreground_process(int status);

    int kill_process(pid_t pid, int sig_num);

    void update_foreground_pid(pid_t pid);

    void addJob(ExternalCommand *cmd, bool isStopped = false);

    void addJob(const char *cmd_line, pid_t pid, bool isStopped = false);

    void printPrompt();

    static const char* SIGKILL_STRING_MESSAGE_1;

    static const char* SIGKILL_STRING_MESSAGE_2;

};

extern SmallShell& SHELL_INSTANCE;

#endif //SMALLSHELL_H
