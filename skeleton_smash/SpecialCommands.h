//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef SPECIALCOMMANDS_H
#define SPECIALCOMMANDS_H
#include "SmallShellHeaders.h"


class SpecialCommand : public Command {
private:

public:
    SpecialCommand() = default;

    SpecialCommand(const char *cmd_line);

    SpecialCommand(const argv& args, const char *cmd_line);

    virtual ~SpecialCommand() = default;

    virtual void execute();

};

class RedirectionCommand : public SpecialCommand {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(argv args, const char* cmd_line);

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public SpecialCommand {
    // TODO: Add your data members
public:
    PipeCommand(argv args, const char* cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public SpecialCommand {
    bool pathGiven;
    bool tooManyArgs;
    string path;
public:
    DiskUsageCommand(argv args, const char* cmd_line);

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
    bool directoryExists(const string& path);
    int calculateDiskUsage(const string& path);
};

class WhoAmICommand : public SpecialCommand {
public:
    WhoAmICommand(argv args, const char* cmd_line);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public SpecialCommand {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(argv args, const char* cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};




#endif //SPECIALCOMMANDS_H
