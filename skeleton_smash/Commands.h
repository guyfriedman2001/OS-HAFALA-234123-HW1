// Ver: 10-4-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include "SmallShellHeaders.h"
typedef std::vector<std::string> argv;

// ########################## NOTE: AbstractCommands code area V ##########################

class Command {
    // TODO: Add your data members
    // maximum_stack_of_current_jobs_ids
    // binary search tree of running jobs
public:
    Command() = default;

    Command(const char *cmd_line);

    Command(char **args, SmallShell& shell); //TODO: CHEACK IF WE CAN DELETE THIS CONSTRUCTOR (DONT USE IT IF YOU HAVENT ALREADY)

    Command(const argv& args, const char *cmd_line);

    virtual ~Command() = default;

    virtual void execute() = 0;

    inline static pid_t getPID();

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class EmptyCommand : public Command { //will go with "Command" and Command404not found in the same file
public:
    EmptyCommand() = default;

    EmptyCommand(const char *cmd_line) : EmptyCommand() {}

    EmptyCommand(const argv& args, const char *cmd_line) : EmptyCommand(cmd_line) {}

    virtual ~EmptyCommand() = default;

    virtual void execute() override {};//printf("\n");};

};

class CommandNotFound : public Command {
public:
    explicit CommandNotFound(const argv& args);

    //CommandNotFound(char **args, int num_args, const char* cmd_line); //std::string first_arg,int num_args, std::string cmd_s

    CommandNotFound(const argv& args, const char* cmd_line);

    virtual ~CommandNotFound() = default;

    void execute() override;
};

// ########################## NOTE: AbstractCommands code area ^ ##########################










// ########################## NOTE: Functor code area V ##########################

class Functor {
    public:
    Functor() = default;
    virtual ~Functor() = default;
    virtual void operator()() = 0;
};

// ########################## NOTE: Functor code area ^ ##########################







#endif //SMASH_COMMAND_H_
