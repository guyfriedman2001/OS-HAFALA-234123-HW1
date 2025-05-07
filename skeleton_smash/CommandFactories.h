//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef COMMANDFACTORIES_H
#define COMMANDFACTORIES_H
#include "SmallShellHeaders.h"
#include "Commands.h"
#include "BuiltInCommands.h"


class SpecialCommand;  

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
    /*inline DerivedClass* makeCommand(argv args, const char* cmd_line){//, SmallShell& shell){
        return dynamic_cast<DerivedClass*>(this->factoryHelper(args,cmd_line));//, shell));
    }*/

    inline Command* makeCommand(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases) {
    return this->factoryHelper(args, cmd_line_after_aliases, cmd_line_before_aliases);
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
    virtual Command* factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases) = 0;
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
    //inline virtual Command* factoryHelper(argv args, const char* cmd_line) override;
    virtual Command* factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);
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
    virtual Command* factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);
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
class SpecialCommandFactory : public CommandFactory<SpecialCommand> {
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
        virtual Command* factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);
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
        virtual Command* factoryHelper(argv args, const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);
};




#endif //COMMANDFACTORIES_H
