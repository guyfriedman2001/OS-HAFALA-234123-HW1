//
// Created by Guy Friedman on 20/04/2025.
//

#include "SpecialCommands.h"
#include "SmallShellHeaders.h"
#include <sys/syscall.h>
#include "CommandFactories.h"
#include "ExternalCommands.h"
#include "SmallShell.h"
#include <cmath>

struct linux_dirent64 {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

SpecialCommand::~SpecialCommand() {}

#if !OLD_CODE_AND_IRRELEVANT
IORedirection::IORedirection(const argv& args, const char *cmd_line) : pid(getpid()), args(args)
{
  assert_not_empty(args);
  strcpy(this->cmd_line, cmd_line);
  m_open_flag = ERR_ARG; //assign default value
}

IORedirection::~IORedirection(){}

int RedirectionCommand::get_first_redirection_index(const argv& args) {
  int returnindex = 0;
  for (const auto& arg : args) {
    if(getOpenFlag(arg) != ERR_ARG) {
      return returnindex;
    }
    ++returnindex;
  }
  return -1;
}

RedirectionCommand::RedirectionCommand(const argv& args, const char *cmd_line) : IORedirection(args, cmd_line)
{
  int redirection_index = get_first_redirection_index(args);
  assert(redirection_index >= 1); //TODO: make sure that we dont get commands with redirection parameters as the first arguments
  assert(redirection_index < args.size() - 1); //redirection arg cannot be the last argumnet
  this->m_open_flag = getOpenFlag(args[redirection_index]);


  //TODO
}

open_flag RedirectionCommand::getOpenFlag(const std::string& arg) {
  if (STRINGS_EQUAL(arg, "<")) {
    return O_RDONLY;
  } else if (STRINGS_EQUAL(arg, ">")) {
    return O_WRONLY | O_CREAT | O_TRUNC;
  } else if (STRINGS_EQUAL(arg, ">>")) {
    return O_WRONLY | O_CREAT | O_APPEND;
  } else if (STRINGS_EQUAL(arg, "<<")) {
    return O_RDONLY;
  } else { //wrong argument given
    return ERR_ARG;
  }
}

void RedirectionCommand::execute()
{
/*
  pid_t pid = fork();
  if (pid == -1) {
      perror("fork failed");
      return;
  }

  if (pid == 0) {
      // Child process

      // 1. Open the file for writing
      int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd == -1) {
          perror("open failed");
          exit(1);
      }

      // 2. Redirect stdout to the file
      if (dup2(fd, STDOUT_FILENO) == -1) {
          perror("dup2 failed");
          close(fd);
          exit(1);
      }

      close(fd); // not needed anymore

      // 3. Execute the command
      char* argv[] = { (char*)"./a.out", nullptr };
      execvp(argv[0], argv);

      // if execvp fails
      perror("execvp failed");
      exit(1);
  } else {
      // Parent process
      int status;
      if (waitpid(pid, &status, 0) == -1) {
          perror("waitpid failed");
      }
  }

 */
  //TODO

  int redirection_index = get_first_redirection_index(args);
  assert(redirection_index >= 1);

  const std::string& filename = args[redirection_index + 1]; // the file after >, >>, etc.

  // Step 1: Backup stdin or stdout
  int backup_fd;
  if (m_open_flag == O_RDONLY || m_open_flag == (O_RDONLY)) { // input redirection (< or <<)
    backup_fd = dup(STDIN_FILENO);
  } else { // output redirection (> or >>)
    backup_fd = dup(STDOUT_FILENO);
  }
  if (backup_fd == -1) {
    perror("dup failed");
    FOR_DEBUG_MODE(cout << "'void RedirectionCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
    return;
  }

  // Step 2: Open file
  int file_fd = open(filename.c_str(), m_open_flag, 0644);
  if (file_fd == -1) {
    perror("open failed");
    FOR_DEBUG_MODE(cout << "'void RedirectionCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
    close(backup_fd);
    return;
  }

  // Step 3: Redirect
  if (m_open_flag == O_RDONLY) {
    if (dup2(file_fd, STDIN_FILENO) == -1) {
      perror("dup2 failed");
      FOR_DEBUG_MODE(cout << "'void RedirectionCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
      close(file_fd);
      close(backup_fd);
      return;
    }
  } else { // output redirection
    if (dup2(file_fd, STDOUT_FILENO) == -1) {
      perror("dup2 failed");
      FOR_DEBUG_MODE(cout << "'void RedirectionCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
      close(file_fd);
      close(backup_fd);
      return;
    }
  }
  close(file_fd); // not needed anymore after dup2

  // Step 4: Execute the real command
  // Remove the redirection part ("> filename") from args
  argv command_args(args.begin(), args.begin() + redirection_index);

  // Rebuild command line
  std::string real_cmd_line;
  for (const auto& arg : command_args) {
    real_cmd_line += arg + " ";
  }

  // Now execute
  argv args = argv(); // FIXME: after we make a function to return argv after aliasing, add call to that function @here
  ExternalCommandFactory factory;
  //ExternalCommand* command_to_execute = factory.makeCommand(move(args), this->cmd_line); //fixme: need to create argv from cmd_line and send both arguments to the factory
  Command* base = factory.makeCommand(std::move(args), this->cmd_line);
  ExternalCommand* command_to_execute = dynamic_cast<ExternalCommand*>(base);
  command_to_execute->execute();
  delete command_to_execute;

  // Step 5: Restore original stdin or stdout
  if (m_open_flag == O_RDONLY) {
    dup2(backup_fd, STDIN_FILENO);
  } else {
    dup2(backup_fd, STDOUT_FILENO);
  }
  close(backup_fd);


  /*//The chat said that this function should be implemented like this, using fork() and such:

  // Step 1: Find redirection operator index (e.g. >, >>, <)
  int redirection_index = get_first_redirection_index(args);
  assert(redirection_index >= 1); // redirection cannot be the first arg

  // Step 2: Extract filename after redirection
  const std::string& filename = args[redirection_index + 1];

  // Step 3: Build argv for the real command (before the redirection operator)
  argv command_args(args.begin(), args.begin() + redirection_index);

  // Step 4: Reconstruct command line string
  std::string real_cmd_line;
  for (const auto& arg : command_args) {
    real_cmd_line += arg + " ";
  }

  // Step 5: Fork a child to handle redirection and execution
  pid_t pid = fork();
  if (pid == -1) {
    perror("smash error: fork failed");
    return;
  }

  if (pid == 0) {
    // === Child process ===

    // Step 6: Separate process group (avoid signals from smash shell)
    setpgrp();

    // Step 7: Open the file for input/output redirection
    int file_fd = open(filename.c_str(), m_open_flag, 0644);
    if (file_fd == -1) {
      perror("smash error: open failed");
      exit(1);
    }

    // Step 8: Perform the actual redirection using dup2
    if (m_open_flag == O_RDONLY) {
      if (dup2(file_fd, STDIN_FILENO) == -1) {
        perror("smash error: dup2 failed");
        close(file_fd);
        exit(1);
      }
    } else { // Output redirection
      if (dup2(file_fd, STDOUT_FILENO) == -1) {
        perror("smash error: dup2 failed");
        close(file_fd);
        exit(1);
      }
    }
    close(file_fd); // Step 9: File descriptor no longer needed after dup2

    // Step 10: Create and execute the real command
    ExternalCommandFactory factory;
    Command* base = factory.makeCommand(command_args, this->cmd_line);
    ExternalCommand* command_to_execute = dynamic_cast<ExternalCommand*>(base);
    command_to_execute->execute();

    // Step 11: If we got here, exec failed inside execute()
    perror("smash error: exec failed");
    exit(1);
  }

  // Step 12: Parent process waits for child to finish
  waitpid(pid, nullptr, 0);*/
}

PipeCommand::PipeCommand(const argv& args, const char *cmd_line)
    : IORedirection(args, cmd_line)
{
  // Find the pipe symbol
  int pipe_index = -1;
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "|" || args[i] == "|&") {
      pipe_index = i;
      is_stderr_pipe = (args[i] == "|&");
      break;
    }
  }
  assert(pipe_index != -1); // must have | or |&

  // Split left and right commands
  left_args = argv(args.begin(), args.begin() + pipe_index);
  right_args = argv(args.begin() + pipe_index + 1, args.end());
}

int PipeCommand::get_first_redirection_index(const argv& args) {
  return -1;
}

void PipeCommand::execute()
{
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe failed");
        FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
        return;
    }

    pid_t left_pid = fork();
    if (left_pid == -1) {
        perror("fork failed (left)");
        FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
        return;
    }

    if (left_pid == 0) {
        // Left child
        setpgrp(); // make sure child is separate if needed

        close(fds[0]); // Close unused read end
        if (is_stderr_pipe) {
            if (dup2(fds[1], STDERR_FILENO) == -1) {
                perror("dup2 failed");
                FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
                exit(1);
            }
        } else {
            if (dup2(fds[1], STDOUT_FILENO) == -1) {
                perror("dup2 failed");
                FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
                exit(1);
            }
        }
        close(fds[1]); // Close after dup2

        // Execute left command
        std::string left_cmd_line;
        for (const auto& arg : left_args) {
            left_cmd_line += arg + " ";
        }
        ExternalCommand(left_cmd_line.c_str()).execute(); // or whatever you use //FIXME: make call to all commands factory
        exit(1); // If exec fails
    }

    pid_t right_pid = fork();
    if (right_pid == -1) {
        perror("fork failed (right)");
        FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
        return;
    }

    if (right_pid == 0) {
        // Right child
        setpgrp(); // again, separate if needed

        close(fds[1]); // Close unused write end
        if (dup2(fds[0], STDIN_FILENO) == -1) {
            perror("dup2 failed");
            FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
            exit(1);
        }
        close(fds[0]); // Close after dup2

        // Execute right command
        string right_cmd_line;
        for (const auto& arg : right_args) {
            right_cmd_line += arg + " ";
        }
        ExternalCommand(right_cmd_line.c_str()).execute(); // or whatever you use  //FIXME: make call to all commands factory
        exit(1); // If exec fails
    }

    // Parent process
    close(fds[0]);
    close(fds[1]);

    // Wait for both children
    int status;
    if (SYSTEM_CALL_FAILED(waitpid(left_pid, &status, Block_until_the_child_terminates))) {
        perror("waitpid (left child) failed");
        FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
    }
    if (SYSTEM_CALL_FAILED(waitpid(right_pid, &status, Block_until_the_child_terminates))) {
        perror("waitpid (right child) failed");
        FOR_DEBUG_MODE(cout << "'void PipeCommand::execute()' at line: " << __LINE__ << " file: " << __FILE__ << endl; )
    }
}

#endif

const char* DiskUsageCommand::TOO_MANY_ARGS = "smash error: du: too many arguments";
const char* DiskUsageCommand::DIRECTORY_DOESNT_EXIST_1 = "smash error: du: directory ";
const char* DiskUsageCommand::DIRECTORY_DOESNT_EXIST_2 = " does not exist";
const char* DiskUsageCommand::TOTAL_DISK_USAGE = "Total disk usage: ";

DiskUsageCommand::DiskUsageCommand(const argv& args, const char *cmd_line, const char *unused_in_special_commands)
{
  if (args.size() == 1)
  {
    pathGiven = false;
    path = getCurrentDirectory();
  } else if (args.size() == 2)
  {
    pathGiven = true;
    path = args[1];
  }
  else
  {
    tooManyArgs = true;
  }
}

void DiskUsageCommand::execute()
{
  if (tooManyArgs)
  {
    cerr << this->TOO_MANY_ARGS << endl;
    return;
  } else if (!pathGiven)
  {
    cout << this->TOTAL_DISK_USAGE << ceil((double)calculateDiskUsage(getCurrentDirectory()) / 1024) << " KB" << endl;
  } else if (directoryExists(path))
  {
    cout << this->TOTAL_DISK_USAGE << ceil((double)calculateDiskUsage(path) / 1024) << " KB" << endl;
  } else {
    cerr << this->DIRECTORY_DOESNT_EXIST_1 << path << this->DIRECTORY_DOESNT_EXIST_2 << endl;
  }
  
    
  
}

bool DiskUsageCommand::directoryExists(const string &path)
{
    int fd; 
    TRY_SYS3(fd, open(path.c_str(), O_RDONLY | O_DIRECTORY), "open");
    if (fd == -1) {
        return false;  
    }
    TRY_SYS3(fd,close(fd),"close");    
    return true;     
}

int DiskUsageCommand::calculateDiskUsage(const string &path)
{
   int totalSize = getFileSize(path);  

    int fd; 
    TRY_SYS3(fd, open(path.c_str(), O_RDONLY | O_DIRECTORY), "open");

    char buffer[4000] = {0};
    struct linux_dirent64* entry;
    int bytesRead;
    TRY_SYS3(bytesRead, syscall(SYS_getdents64, fd, buffer, sizeof(buffer)), "getdents64");
    while (bytesRead > 0) {
        int offset = 0;
        while (offset < bytesRead) {
            entry = (struct linux_dirent64*)(buffer + offset);  
            string itemName = entry->d_name;
            if (itemName == "." || itemName == "..") {
                offset += entry->d_reclen;
                continue;
            }
            string fullPath = path + "/" + itemName;
            if (directoryExists(fullPath)) {
                totalSize += calculateDiskUsage(fullPath); 
            } else {
                totalSize += getFileSize(fullPath);
            }
            offset += entry->d_reclen;
        }
        TRY_SYS3(bytesRead, syscall(SYS_getdents64, fd, buffer, sizeof(buffer)), "getdents64");
    }

    TRY_SYS3(fd,close(fd),"close");
    return totalSize;
}

string DiskUsageCommand::getCurrentDirectory()
{
    SmallShell::getInstance().tryLoadShellPath(this->buffer, sizeof(this->buffer));
    return this->buffer;
}

int DiskUsageCommand::getFileSize(const string& path) 
{
    struct stat st;
    if (lstat(path.c_str(), &st) == 0) {
        return st.st_blocks * 512;
    }
    return 0;
}

WhoAmICommand::WhoAmICommand(const argv& args,  const char *cmd_line, const char *unused_in_special_commands)
{
  uid = findUID();
  username = findUsername(uid);
  homeDirectory = findHomeDirectory(uid);
}

void WhoAmICommand::execute()
{
  cout << username << homeDirectory << endl;
}

int WhoAmICommand::findUID() //TODO add TRY_SYS2 where needed 
{
    
    int fd;
    TRY_SYS3(fd, open("/proc/self/status", O_RDONLY), "open");
    if (fd < 0) {
       return -1; 
    }
    char buffer[4096];
    ssize_t bytesRead;
    TRY_SYS3(bytesRead ,read(fd, buffer, sizeof(buffer) - 1), "read");
    TRY_SYS3(fd,close(fd), "close");
    if (bytesRead <= 0) {
       return -1;
    }
    buffer[bytesRead] = '\0';
    
    char* start = strstr(buffer, "Uid:");
    if (!start) {
       return -1; 
    }

    start += 4; // Skip "Uid:"
    while (*start == ' ' || *start == '\t') start++; //skip spaces and tabs
    return atoi(start);
}

string WhoAmICommand::findHomeDirectory(int uid)
{
    return getFieldByUid(uid, 5);
}

string WhoAmICommand::findUsername(int uid) //TODO add TRY_SYS2 where needed 
{
    return getFieldByUid(uid, 0);
}

string WhoAmICommand::getFieldByUid(int uid, int fieldIndex) {
    int fd;
    TRY_SYS3(fd,open("/etc/passwd", O_RDONLY),"open");
    if (fd < 0) {
      return "";
    }

    char buffer[4096];
    string line;
    ssize_t bytesRead;

    TRY_SYS3(bytesRead , read(fd, buffer, sizeof(buffer) - 1), "read");
    while (bytesRead > 0) {
        for (int i = 0; i < bytesRead; ++i) {
            char character = buffer[i];
            if (character == '\n') {
                std::stringstream ss(line);
                string field;
                argv fields;

                while (std::getline(ss, field, ':')) {
                    fields.push_back(field);
                }

                if (fields.size() > fieldIndex && fields.size() >= 3) { //ensures 3 fields: username, password, uid
                    int parsedUid = stoi(fields[2]);
                    if (parsedUid == uid) { //ensures the uid is correct
                        TRY_SYS3(fd,close(fd),"close");
                        return fields[fieldIndex];
                    }
                }
                line.clear();
            } else {
                line += character;
            }
        }
    }
    TRY_SYS3(fd,close(fd),"close");
    return "";
}

const char* NetInfo::NOT_SPECIFIED = "smash error: netinfo: interface not specified";
const char* NetInfo::INTERFACE_DOESNT_EXIST_1 = "smash error: netinfo: interface ";
const char* NetInfo::INTERFACE_DOESNT_EXIST_2 = " does not exist";

NetInfo::NetInfo(const argv& args, const char *cmd_line, const char *unused_in_special_commands)
{
  iPAdress = getIPAddress(args[1]);
  subnetMask = getSubnetMask(args[1]);
  
}

void NetInfo::execute()
{
  if (iPAdress.empty() || subnetMask.empty()) {
    cerr << this->NOT_SPECIFIED << endl;
    return;
  }
  cout << "IP Address: " << iPAdress << endl;
  cout << "Subnet Mask: " << subnetMask << endl;
}

string getInterfaceAddress(const string& interfaceName, int ioctlCommand) {
    int networkSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (networkSocket < 0) {
        return "";
    }
    struct ifreq interfaceRequest = {};
    strncpy(interfaceRequest.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    interfaceRequest.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(networkSocket, ioctlCommand, &interfaceRequest) < 0) {
        close(networkSocket);
        return "";
    }
    struct sockaddr_in* address_struct = (struct sockaddr_in*)&interfaceRequest.ifr_addr;
    string result = inet_ntoa(address_struct->sin_addr);

    close(networkSocket);
    return result;
}

string NetInfo::getIPAddress(const string& interfaceName)
{
    return getInterfaceAddress(interfaceName, SIOCGIFADDR);
}

string NetInfo::getSubnetMask(const string& interfaceName)
{
    return getInterfaceAddress(interfaceName, SIOCGIFNETMASK);
}

string NetInfo::getDefaultGetway(const string &interfaceName)
{
    int fd;
    TRY_SYS3(fd, open("/proc/net/route", O_RDONLY), "open");
    if (fd < 0) {
        return "";
    }
    char buffer[4096];
    ssize_t bytesRead;
    TRY_SYS3(fd, read(fd, buffer, sizeof(buffer) - 1), "read");
    TRY_SYS3(fd,close(fd), "close");
    if (bytesRead <= 0) {
        return "";
    }
    buffer[bytesRead] = '\0';

    string bufferString(buffer);
    istringstream iss(bufferString);
    string line;
    getline(iss, line);
    while (getline(iss,line)) {
        string gateway = extractGateway(line, interfaceName);
        if (!gateway.empty()) {
            return gateway;
        }
    }

    return "";
}

string extractGateway(const string& line, const string& interfaceName) {
    istringstream iss(line);
    string iface, destHex, gatewayHex;
    iss >> iface >> destHex >> gatewayHex;
    if (iface == interfaceName && destHex == "00000000") {
        unsigned long actualGetaway;
        std::stringstream convert;
        convert << std::hex << gatewayHex;
        convert >> actualGetaway;

        struct in_addr gatewayAddress;
        gatewayAddress.s_addr = actualGetaway;
        return inet_ntoa(gatewayAddress);
    }
    return "";
}

string NetInfo::getDnsServers()
{
    return string();
}
