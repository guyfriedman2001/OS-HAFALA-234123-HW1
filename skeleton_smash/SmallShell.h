//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef SMALLSHELL_H
#define SMALLSHELL_H
#include "SmallShellHeaders.h"


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
    inline char* loadShellPath(char* buffer_location, size_t buffer_size);
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

    inline static std::string getDefaultPrompt();

    inline void changePrompt(std::string nextPrompt);

    inline int getPID();

    inline void tryLoadShellPath(char* buffer_location, size_t buffer_size);

    inline void updateOldPath();

    inline bool hasOldPath();

    inline string getPreviousPath();

    inline bool changeShellDirectory(const char* next_dir);

    inline std::string getPrompt();

    inline std::string getEndStr();

    inline void print_current_path() const;

    inline JobsList& getJobsList();

    inline AliasManager& getAliases();

    inline void print_jobs();

    inline int waitPID(pid_t pid);

    inline JobsList::JobEntry *getJobById(int jobId);

    inline int get_max_current_jobID();

    inline pid_t get_foreground_pid();

    inline int kill_foreground_process(int status);

    inline int kill_process(pid_t pid, int sig_num);

    inline void update_foreground_pid(pid_t pid);

    inline void addJob(ExternalCommand *cmd, bool isStopped = false);

    inline void addJob(const char *cmd_line, pid_t pid, bool isStopped = false);

};



#endif //SMALLSHELL_H
