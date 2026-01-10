//
// Created by Guy Friedman on 20/04/2025.
//

#ifndef JOBLIST_H
#define JOBLIST_H
#include "SmallShellHeaders.h"
//#include "ExternalCommands.h" 

class ExternalCommand;

class JobsList {
public:
    class JobEntry {
    private:
        ExternalCommand* command;
        //char* cmd_line;
        int jobID;
        //int jobPID;
    public:
        JobEntry(ExternalCommand* command, int jobID);

        //JobEntry(ExternalCommand* command, int jobID, int JobPID) : JobEntry(command, jobID) { this->jobPID = jobPID; }

        ~JobEntry() = default;

        inline void printYourself();

        inline pid_t getJobPID() const ; //get the PID of the running command

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

    //JobEntry *getLastJob(int *lastJobId);

    //JobEntry *getLastStoppedJob(int *jobId);

    int get_max_current_jobID();

    int numberOfJobs();

    void sendSignalToJobById(int pidToSendTo, int signalToSend);

    inline pid_t getJobPID(int jobID); 

    // TODO: Add extra methods or modify exisitng ones as needed
};



#endif //JOBLIST_H
