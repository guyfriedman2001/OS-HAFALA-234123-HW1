// Ver: 10-4-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    // TODO: Add your data members
    // maximum_stack_of_current_jobs_ids
    // binary search tree of running jobs
public:
    Command(const char *cmd_line);

    virtual ~Command();

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line);

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);

    virtual ~ExternalCommand() {
    }

    void execute() override;
};

/**
 * @brief A generic abstract factory class for creating Command objects from parsed arguments.
 *
 * This templated class defines a static interface for instantiating specific command types
 * from a pre-parsed list of C-style string arguments (as produced by _parseCommandLine).
 *
 * @tparam DerivedClass The specific subclass of Command to be created (e.g., BuiltInCommand or ExternalCommand).
 *
 * Usage:
 *   - Clients should call `makeCommand(char** args)` to create a Command instance.
 *   - Subclasses must implement the `factoryHelper` function, which handles the actual construction logic.
 *
 * Notes:
 *   - The constructor is deleted to enforce static usage.
 *   - The arguments must be null-terminated and pre-parsed (e.g., by `_parseCommandLine`).
 *   - The caller is responsible for managing the memory of the returned Command object.
 */
template <typename DerivedClass>
class CommandFactory{
public:
    CommandFactory() = delete;
    virtual ~CommandFactory() = default;

     /**
     * @brief Creates a new Command instance using a subclass-defined factory method.
     *
     * @param args A null-terminated array of C-style strings representing the command and its arguments.
     * @return A pointer to the created DerivedClass command object.
     */
    inline static DerivedClass* makeCommand(char **args, SmallShell& shell){
        return dynamic_cast<DerivedClass*>(factoryHelper(args, shell));
    }

protected:

    /**
     * @brief Subclass-specific method for constructing a Command from parsed arguments.
     *
     * Must be overridden in subclasses to perform command-specific instantiation logic.
     *
     * @param args A null-terminated array of C-style strings.
     * @return A pointer to a newly constructed Command object.
     */
    inline virtual Command* factoryHelper(char **args, SmallShell& shell) = 0;
};

/**
 * @brief A concrete factory for creating BuiltInCommand instances from parsed arguments.
 *
 * Implements the factory logic for interpreting built-in shell commands (e.g., "cd", "exit").
 *
 * Notes:
 *   - Returns nullptr in case of failure (or when the supplied command is not from a BuiltInCommand type)
 *   - It expects the input `args` to be a null-terminated array as parsed by `_parseCommandLine`.
 */
class BuiltInCommandFactory : public CommandFactory<BuiltInCommand> {
public:
    BuiltInCommandFactory() = delete;
    virtual ~BuiltInCommandFactory() = default;
protected:

    /**
     * @brief Creates a BuiltInCommand instance using the provided arguments.
     *
     * @param args A null-terminated array of C-style strings.
     * @return A pointer to a new BuiltInCommand object.
     */
    inline virtual Command* factoryHelper(char **args, SmallShell& shell) override;
};

/**
 * @brief A concrete factory for creating ExternalCommand instances from parsed arguments.
 *
 * This factory constructs command objects for external system commands that are not part
 * of the shell's built-in functionality.
 *
 * Notes:
 *   - Returns nullptr in case of failure (or when the supplied command is not from an ExternalCommand type)
 *   - It should be used for any command that must be executed via a system call (e.g., using `exec`).
 *   - Input `args` must be a null-terminated array produced by `_parseCommandLine`.
 */
class ExternalCommandFactory : public CommandFactory<ExternalCommand> {
public:
    ExternalCommandFactory() = delete;
    virtual ~ExternalCommandFactory() = default;
protected:

    /**
     * @brief Creates an ExternalCommand instance using the provided arguments.
     *
     * @param args A null-terminated array of C-style strings.
     * @return A pointer to a new ExternalCommand object.
     */
    inline virtual Command* factoryHelper(char **args, SmallShell& shell) override;
};

/**
 * @brief A factory for creating Command instances based on parsed arguments.
 *
 * This factory constructs command objects for commands that are not part of the
 * shell's built-in functionality, returning different command types based on 
 * the provided arguments.
 *
 * Notes:
 *   - Returns nullptr in case of failure (or when the supplied command is not recognized).
 *   - This should be used for any command that can be identified and created dynamically.
 *   - Input `args` must be a null-terminated array produced by `_parseCommandLine`.
 */
class SpecialCommandFactory : public CommandFactory<Command> {
    public:
        SpecialCommandFactory() = delete;
        virtual ~SpecialCommandFactory() = default;
    
    protected:
        /**
         * @brief Creates a Command instance using the provided arguments.
         *
         * @param args A null-terminated array of C-style strings.
         * @return A pointer to a new Command object, or nullptr if no valid command is found.
         */
        inline virtual Command* factoryHelper(char **args, SmallShell& shell) override;
};    

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(const char *cmd_line);

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(const char *cmd_line);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(const char *cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    ChangeDirCommand(const char *cmd_line, char **plastPwd);

    virtual ~ChangeDirCommand() {
    }

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {
    }

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand() {
    }

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {
    }

    void execute() override;
};


class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    };

    // TODO: Add your data members
public:
    JobsList();

    ~JobsList();

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand {
public:
    AliasCommand(const char *cmd_line);

    virtual ~AliasCommand() {
    }

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand {
public:
    UnAliasCommand(const char *cmd_line);

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand(const char *cmd_line);

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;
};

class WatchProcCommand : public BuiltInCommand {
public:
    WatchProcCommand(const char *cmd_line);

    virtual ~WatchProcCommand() {
    }

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    SmallShell();

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

    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
