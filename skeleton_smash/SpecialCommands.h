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

    inline pid_t getPID();

};

class IORedirection : public SpecialCommand {
protected:
    pid_t pid;
    argv args;
    char cmd_line[COMMAND_MAX_LENGTH];
    open_flag m_open_flag;
public:
    explicit IORedirection(const argv& args, const char* cmd_line);

    virtual ~IORedirection() {}

    void execute() override;

    //open_flag getOpenFlag(const string& arg);

    virtual inline pid_t create_fork();

    virtual inline int get_first_redirection_index(const argv& args) = 0;
};

class RedirectionCommand : public IORedirection {

public:
    explicit RedirectionCommand(const argv& args, const char* cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;

    inline static open_flag getOpenFlag(const string& arg);

    virtual inline int get_first_redirection_index(const argv& args) override;
};

class PipeCommand : public IORedirection {
private:
    argv left_args;
    argv right_args;
    bool is_stderr_pipe = false;
public:
    PipeCommand(const argv& args, const char* cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public SpecialCommand {
    bool pathGiven;
    bool tooManyArgs;
    string path;
    char buffer[MAX_DIR_LENGTH];
    const static char* TOO_MANY_ARGS;
    const static char* DIRECTORY_DOESNT_EXIST_1;
    const static char* DIRECTORY_DOESNT_EXIST_2;
    const static char* TOTAL_DISK_USAGE;

public:
    DiskUsageCommand(const argv& args, const char* cmd_line);

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
    bool directoryExists(const string& path);
    int calculateDiskUsage(const string& path);
    string getCurrentDirectory();
    int getFileSize(const string& path);
};

class WhoAmICommand : public SpecialCommand {
    string username;
    string homeDirectory;
public:
    WhoAmICommand(const argv& args, const char* cmd_line);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public SpecialCommand {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(const argv& args, const char* cmd_line);

    virtual ~NetInfo() {
    }

    void execute() override;
};




#endif //SPECIALCOMMANDS_H
