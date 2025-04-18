// Ver: 10-4-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cassert>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define MAX_DIR_LENGTH (COMMAND_MAX_LENGTH)

using std::string;

typedef std::vector<std::string> argv;


class SmallShell;








// ########################## NOTE: AbstractCommands code area V ##########################

class Command {
    // TODO: Add your data members
    // maximum_stack_of_current_jobs_ids
    // binary search tree of running jobs
public:
    Command() = default;

    Command(const char *cmd_line);

    Command(char **args, SmallShell& shell);

    virtual ~Command() = default;

    virtual void execute() = 0;

    inline int getPID();

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand() = default;
    
    BuiltInCommand(const char *cmd_line);

    BuiltInCommand(char **args, SmallShell& shell);

    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
private:
    char command[COMMAND_MAX_LENGTH];
public:
    ExternalCommand(const char *cmd_line);

    ExternalCommand(const argv& args,const char *cmd_line);

    virtual ~ExternalCommand() {
    }

    void execute() override;

    inline void printYourself();

    inline int getPID();
};

// ########################## NOTE: AbstractCommands code area ^ ##########################










// ########################## NOTE: CommandFactory code area V ##########################

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
    CommandFactory() = default;
    virtual ~CommandFactory() = default;

     /**
     * @brief Creates a new Command instance using a subclass-defined factory method.
     *
     * @param args A null-terminated array of C-style strings representing the command and its arguments.
     * @return A pointer to the created DerivedClass command object.
     */
    inline DerivedClass* makeCommand(argv args, const char* cmd_line){//, SmallShell& shell){
        return dynamic_cast<DerivedClass*>(this->factoryHelper(args,cmd_line));//, shell));
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
    inline virtual Command* factoryHelper(argv args, const char* cmd_line) = 0;
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
    BuiltInCommandFactory() = default;
    virtual ~BuiltInCommandFactory() = default;
protected:

    /**
     * @brief Creates a BuiltInCommand instance using the provided arguments.
     *
     * @param args A null-terminated array of C-style strings.
     * @return A pointer to a new BuiltInCommand object.
     */
    inline virtual Command* factoryHelper(argv args, const char* cmd_line) override;
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
    ExternalCommandFactory() = default;
    virtual ~ExternalCommandFactory() = default;
protected:

    /**
     * @brief Creates an ExternalCommand instance using the provided arguments.
     *
     * @param args A null-terminated array of C-style strings.
     * @return A pointer to a new ExternalCommand object.
     */
    inline virtual Command* factoryHelper(argv args, const char* cmd_line) override;
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
        SpecialCommandFactory() = default;
        virtual ~SpecialCommandFactory() = default;
    
    protected:
        /**
         * @brief Creates a Command instance using the provided arguments.
         *
         * @param args A null-terminated array of C-style strings.
         * @return A pointer to a new Command object, or nullptr if no valid command is found.
         */
        inline virtual Command* factoryHelper(argv args, const char* cmd_line) override;
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
class Error404CommandNotFound : public CommandFactory<Command> {
    public:
    Error404CommandNotFound() = default;
        virtual ~Error404CommandNotFound() = default;
    
    protected:
        /**
         * @brief Creates a Command instance using the provided arguments.
         *
         * @param args A null-terminated array of C-style strings.
         * @return A pointer to a new Command object, or nullptr if no valid command is found.
         */
        inline virtual Command* factoryHelper(argv args, const char* cmd_line) override;
};   

// ########################## NOTE: CommandFactory code area ^ ##########################










// ########################## NOTE: SpecialCommands code area V ##########################

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(argv args, const char* cmd_line);

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(argv args, const char* cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(argv args, const char* cmd_line);

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(argv args, const char* cmd_line);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(argv args, const char* cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};

// ########################## NOTE: SpecialCommands code area ^ ##########################










// ########################## NOTE: JobHandling code area V ##########################

class JobsList {
public:
    class JobEntry {
    private:
        ExternalCommand* command;
        //char* cmd_line;
        int jobID;
    public:
        JobEntry(ExternalCommand* command, int jobID);
        ~JobEntry() = default;
        inline void printYourself();
        inline int getJobPID(); //get the PID of the running command

    };
    typedef std::map<int, JobEntry> Jobs;

    Jobs jobs;

    // TODO: Add your data members
public:
    JobsList();

    ~JobsList();

    void addJob(ExternalCommand *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    
    int get_max_current_jobID();

    int numberOfJobs();

    void sendSignalToJobById(int pidToSendTo, int signalToSend); //TODO need to handle signal errors inside - comes later in the HW

    inline int getJobPID(int jobID); //get the PID of the running command of the job with a specifiec ID

    // TODO: Add extra methods or modify exisitng ones as needed
};

// ########################## NOTE: JobHandling code area ^ ##########################










// ########################## NOTE: BuiltInCommand code area V ##########################

class CommandNotFound : public BuiltInCommand {
public:
    explicit CommandNotFound(const argv& args);

    //CommandNotFound(char **args, int num_args, const char* cmd_line); //std::string first_arg,int num_args, std::string cmd_s

    CommandNotFound(const argv& args, const char* cmd_line);

    virtual ~CommandNotFound() = default;
    
    void execute() override;
};

class ChangePromptCommand : public BuiltInCommand {
private:
    std::string nextPrompt;
public:
    explicit ChangePromptCommand(const argv& args);

    //ChangePromptCommand(char **args, int num_args, const char* cmd_line);

    ChangePromptCommand(const argv& args, const char* cmd_line);
    
    virtual ~ChangePromptCommand() = default;
    
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
    private:
        int smashPID;
    public:
        ShowPidCommand();

        explicit ShowPidCommand(const argv& args);

        //ShowPidCommand(char **args, int num_args, const char* cmd_line);

        ShowPidCommand(const argv& args, const char* cmd_line);


        virtual ~ShowPidCommand() = default;
    
        void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand { // AKA "pwd" / "cwd"
private:
    char current_path[MAX_DIR_LENGTH];
public:
    GetCurrDirCommand();

    explicit GetCurrDirCommand(const argv& args);

    //GetCurrDirCommand(char **args, int num_args, const char* cmd_line);

    GetCurrDirCommand(const argv& args, const char* cmd_line);

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

    ChangeDirCommand(const argv& args, const char* cmd_line);

    virtual ~ChangeDirCommand() = default;

    void execute() override;
};

class JobsCommand : public BuiltInCommand {  // AKA "jobs"
    // TODO: Add your data members
public:
    explicit JobsCommand(const argv& args);

    //JobsCommand(char **args, int num_args, const char* cmd_line);

    JobsCommand(const argv& args, const char* cmd_line);

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
    
public:
    //ForegroundCommand(char **args);

    //ForegroundCommand(char **args, int num_args, const char* cmd_line);

    ForegroundCommand(const argv& args, const char* cmd_line);

    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class QuitCommand : public BuiltInCommand { // AKA "quit"
    // TODO: Add your data members 
    bool killSpecified;
public:
    QuitCommand(char **args);

    QuitCommand(argv args, const char* cmd_line);


    virtual ~QuitCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand { // AKA "kill"
    // TODO: Add your data members
    int signalToSend;
    int pidToSendTo;
public:
    KillCommand(char **args);

    KillCommand(argv args, const char* cmd_line);

    virtual ~KillCommand() {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand { // AKA "alias"

    bool aliasList;
    string actualCommand;
    string aliasName;
public:
    AliasCommand(char **args);

    AliasCommand(argv args, const char* cmd_line);

    virtual ~AliasCommand() {
    }
    
    void execute() override;
    string extractAlias(argv args);
    string extractActualCommand(argv args);
};
    
class UnAliasCommand : public BuiltInCommand { // AKA "unalias"
    bool noArgs;
    argv aliasesToRemove;
public:
    UnAliasCommand(char **args);

    UnAliasCommand(argv args, const char* cmd_line);

    virtual ~UnAliasCommand() {
    }
    
    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand { // AKA "unsetenv"
    public:
        UnSetEnvCommand(char **args);

        UnSetEnvCommand(argv args, const char* cmd_line);
    
        virtual ~UnSetEnvCommand() {
        }
    
        void execute() override;
};

class WatchProcCommand : public BuiltInCommand { // AKA "watchproc"
public:
    WatchProcCommand(char **args);

    WatchProcCommand(argv args, const char* cmd_line);
    
    virtual ~WatchProcCommand() {
    }
    
    void execute() override;
};

// ########################## NOTE: BuiltInCommand code area ^ ##########################
















// ########################## NOTE: AliasHandling code area V ##########################

class AliasManager {
    std::unordered_map<string, string> aliases;

    public:

    argv uncoverAlias(argv original);
    void addAlias(const string& newAliasName, string args);
    void removeAlias(const string& aliasToRemove);
    bool isReserved(const string& newAliasName) const;
    bool doesExist(const string& newAliasName) const;
    bool isSyntaxValid(const string& newAliasName) const;
    void printAll() const;
};

// ########################## NOTE: AliasHandling code area ^ ##########################


// ########################## NOTE: SmallShell code area V ##########################

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

    void executeCommand(Command *command);

    inline static std::string getDefaultPrompt();

    inline void changePrompt(std::string nextPrompt);

    inline int getPID();

    inline char* loadShellPath(char* buffer_location, size_t buffer_size);

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



};

// ########################## NOTE: SmallShell code area ^ ##########################




#endif //SMASH_COMMAND_H_
