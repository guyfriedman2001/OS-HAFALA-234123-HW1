//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef SPECIALCOMMANDS_H
#define SPECIALCOMMANDS_H
#include "SmallShellHeaders.h"
#include "Commands.h"

class SpecialCommand : public Command {
private:


public:
    SpecialCommand() = default;

    SpecialCommand(const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    SpecialCommand(const argv& args,  const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~SpecialCommand();

    virtual void execute() = 0;

    pid_t getPID();

};

#define OLD_CODE_AND_IRRELEVANT true
#if !OLD_CODE_AND_IRRELEVANT
class IORedirection : public SpecialCommand {
protected:
    pid_t pid;
    argv args;
    char cmd_line[COMMAND_MAX_LENGTH];
    open_flag m_open_flag;
public:
    explicit IORedirection(const argv& args,  const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~IORedirection();

    void execute() override = 0;

    //open_flag getOpenFlag(const string& arg);

    //virtual inline pid_t create_fork();

    //virtual inline int get_first_redirection_index(const argv& args) = 0;
    virtual int get_first_redirection_index(const argv& args) = 0;
};

class RedirectionCommand : public IORedirection {

public:
    explicit RedirectionCommand(const argv& args,  const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~RedirectionCommand() {}

    void execute() override;

    //inline static open_flag getOpenFlag(const string& arg);
    static open_flag getOpenFlag(const string& arg);

    //virtual inline int get_first_redirection_index(const argv& args) override;
    virtual int get_first_redirection_index(const argv& args) override;
};

class PipeCommand : public IORedirection {
private:
    argv left_args;
    argv right_args;
    bool is_stderr_pipe = false;
public:
    PipeCommand(const argv& args,  const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~PipeCommand() {
    }

    void execute() override;
    virtual int get_first_redirection_index(const argv& args) override;
};
#endif

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
    DiskUsageCommand(const argv& args,  const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
    bool directoryExists(const string& path);
    int calculateDiskUsage(const string& path);
    string getCurrentDirectory();
    int getFileSize(const string& path);
};

class WhoAmICommand : public SpecialCommand {
    int uid;
    string username;
    string homeDirectory;
public:
    WhoAmICommand(const argv& args,  const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
    int findUID();
    string findHomeDirectory(int uid);
    string findUsername(int uid);
    string getFieldByUid(int uid, int fieldIndex);
};

class NetInfo : public SpecialCommand {
    string iPAdress;
    string subnetMask;
    string defaultGetway;
    string dnsServers;
    const static char* NOT_SPECIFIED;
    const static char* INTERFACE_DOESNT_EXIST_1;
    const static char* INTERFACE_DOESNT_EXIST_2;
public:
    NetInfo(const argv& args,  const char *cmd_line_after_aliases, const char *cmd_line_before_aliases);

    virtual ~NetInfo() {
    }

    void execute() override;
    string getInterfaceAdress(const string& interfaceName, int ioctlCommand);
    string getIPAddress(const string& interfaceName);
    string getSubnetMask(const string& interfaceName);
    string getDefaultGetway(const string& interfaceName);
    string extractGateway(const string& line, const string& interfaceName);
    string getDnsServers();
};




#endif //SPECIALCOMMANDS_H
