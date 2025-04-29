//
// Created by Guy Friedman on 20/04/2025.
//

#include "SpecialCommands.h"
#include "SmallShellHeaders.h"
#include <sys/syscall.h>

//#include <linux/dirent.h>
struct linux_dirent64 {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

IORedirection::IORedirection(const argv& args, const char *cmd_line) : pid(getpid()), args(args)
{
  assert_not_empty(args);
  strcpy(this->cmd_line, cmd_line);
  m_open_flag = ERR_ARG; //assign default value
}

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
  Command* base = factory.makeCommand(move(args), this->cmd_line);
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


DiskUsageCommand::DiskUsageCommand(const argv& args, const char *cmd_line)
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
    cerr << this->TOO_MANY_ARGS;
    return;
  } else if (!pathGiven)
  {
    cout << this->TOTAL_DISK_USAGE << calculateDiskUsage(getCurrentDirectory()) << " KB";
  } else if (directoryExists(path))
  {
    cout << this->TOTAL_DISK_USAGE << calculateDiskUsage(path) << " KB";
  } else {
    cerr << this->DIRECTORY_DOESNT_EXIST_1 << path << this->DIRECTORY_DOESNT_EXIST_2;
  }
  
    
  
}

bool DiskUsageCommand::directoryExists(const string &path)
{
    int fd; 
    TRY_SYS2(fd = open(path.c_str(), O_RDONLY | O_DIRECTORY), "open");
    if (fd == -1) {
        return false;  
    }
    TRY_SYS2(close(fd),"close");    
    return true;     
}

int DiskUsageCommand::calculateDiskUsage(const string &path)
{
   int totalSize = getFileSize(path);  

    int fd; 
    TRY_SYS2(fd = open(path.c_str(), O_RDONLY | O_DIRECTORY), "open");

    char buffer[4000] = {0};
    struct linux_dirent64* entry;
    int bytesRead;
    TRY_SYS2(bytesRead = syscall(SYS_getdents64, fd, buffer, sizeof(buffer)), "getdents64");
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
        TRY_SYS2(bytesRead = syscall(SYS_getdents64, fd, buffer, sizeof(buffer)), "getdents64");
    }

    TRY_SYS2(close(fd),"close");
    return totalSize;
}

string DiskUsageCommand::getCurrentDirectory()
{
    SmallShell::getInstance().tryLoadShellPath(this->buffer, sizeof(this->buffer));
    return this->buffer;
}

int getFileSize(const string& path) 
{
    struct stat st;
    if (lstat(path.c_str(), &st) == 0) {
        return st.st_blocks * 512;
    }
    return 0;
}

WhoAmICommand::WhoAmICommand(const argv& args, const char *cmd_line)
{
  //struct passwd* pwd = getpwuid(id);
  //username = pwd->pw_name;
  //homeDirectory = pwd->pw_dir;
}

void WhoAmICommand::execute()
{
  cout << username << "/" << homeDirectory << endl;
}

NetInfo::NetInfo(const argv& args, const char *cmd_line)
{
  //BONUS
}

void NetInfo::execute()
{
  //BONUS
}