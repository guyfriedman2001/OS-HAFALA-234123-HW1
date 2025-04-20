//
// Created by Guy Friedman on 20/04/2025.
//

#include "SpecialCommands.h"
#include "SmallShellHeaders.h"

RedirectionCommand::RedirectionCommand(argv args, const char *cmd_line)
{
  //TODO
}

void RedirectionCommand::execute()
{
  //TODO
}

PipeCommand::PipeCommand(argv args, const char *cmd_line)
{
  //TODO
}

void PipeCommand::execute()
{
  //TODO
}

DiskUsageCommand::DiskUsageCommand(argv args, const char *cmd_line)
{
  if (args.size() == 1)
  {
    pathGiven = false;
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
    std::cerr << "smash error: du: too many arguments" << endl;
    return;
  }


}

bool DiskUsageCommand::directoryExists(const string &path)
{
    return false;
}

int DiskUsageCommand::calculateDiskUsage(const string &path)
{
    int totalSize = 0;

    int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        std::cerr << "Failed to open directory: " << path << std::endl;
        return 0;
    }
    char buffer[4096] = {0};
    int nread;
    struct linux_dirent64* entry;
    // Read directory entries using getdents64
    while ((nread = syscall(SYS_getdents64, fd, buffer, BUF_SIZE)) > 0) {
        int offset = 0;

        // Loop through all directory entries in the buffer
        while (offset < nread) {
            entry = (struct linux_dirent64*)(buf + offset);  // Point to the current entry
            string name = entry->d_name;  // Get the file/directory name
            stringstream s;
            s << path << "/" << name;
            string fullPath = s.str(); // Build full path to the current entry
            struct stat st;
              if (lstat(fullPath.c_str(), &st) == 0) { // gets data about the file or directory
                  if (S_ISDIR(st.st_mode)) { // Check if it's a directory
                        totalSize += calculateDirectorySize(fullPath);
                  }
                  else if (S_ISREG(st.st_mode)) { // Check if it's a regular file
                        totalSize += st.st_size;  // Add file size to total
                  }
              }
            offset += entry->d_reclen;  // Move to the next directory entry
        }
    }

    close(fd);
    return totalSize;
}

WhoAmICommand::WhoAmICommand(argv args, const char *cmd_line)
{
  //TODO
}

void WhoAmICommand::execute()
{
  //TODO
}

NetInfo::NetInfo(argv args, const char *cmd_line)
{
  //BONUS
}

void NetInfo::execute()
{
  //BONUS
}