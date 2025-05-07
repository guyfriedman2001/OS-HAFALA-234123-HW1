//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef EXTERNALCOMMANDS_H
#define EXTERNALCOMMANDS_H
#include "SmallShellHeaders.h"
#include "Commands.h"


class ExternalCommand : public Command {
protected:
    char command_original[COMMAND_MAX_LENGTH];
    argv given_args;
    pid_t jobPID = 0;
public:
    ExternalCommand(const char *cmd_line);

    ExternalCommand(const char *cmd_line, pid_t pid);

    ExternalCommand(const argv& args,const char *cmd_line);

    virtual ~ExternalCommand() {
    }

    virtual void execute() override;

    //inline void printYourself();
    void printYourself();

    //inline int getPID();

    virtual void executeHelper();

    //inline pid_t getPID();
    pid_t getPID();

};

class ComplexExternalCommand : public ExternalCommand {
private:
    //char command[COMMAND_MAX_LENGTH];
    //argv given_args;
public:
    ComplexExternalCommand(const char *cmd_line) : ExternalCommand(cmd_line) {}

    ComplexExternalCommand(const argv& args,const char *cmd_line) : ExternalCommand(args,cmd_line) {}

    virtual ~ComplexExternalCommand() {
    }

    virtual void executeHelper() override;
};



#endif //EXTERNALCOMMANDS_H
