//
// Created by Guy Friedman on 20/04/2025.
//

#include "JobList.h"

#include <signal.h>

#include "SmallShellHeaders.h"
#include "ExternalCommands.h"

JobsList::JobEntry::JobEntry(ExternalCommand *command, int jobID) : command(command), jobID(jobID) {}

void JobsList::JobEntry::printYourself()
{
  printf("[%d] ",this->jobID);
  this->command->printYourself();
}

JobsList::JobsList()
{
  // TODO:
}

JobsList::~JobsList()
{
  // TODO:
}

void JobsList::addJob(ExternalCommand *cmd, bool isStopped)
{
  Jobs &jbs = this->jobs;
  int max_curr_job_id = this->get_max_current_jobID();
  int next_id = ++max_curr_job_id;
  JobEntry toInsert = JobEntry(cmd, next_id);
  jbs.insert(std::make_pair(next_id, toInsert));
}

void JobsList::printJobsList()
{
  for (auto &pair : jobs)
  {
    // int jobId = pair.first;
    JobEntry &job = pair.second;
    job.printYourself();
    printf("\n");
  }
}

#if temporairly_disable_kill_all_jobs
void JobsList::killAllJobs(){}
#elif //if temporairly_disable_kill_all_jobs
void JobsList::killAllJobs()
{
  for (const auto& job : jobs)
  {
    pid_t pid = job.second.getJobPID();
    if (pid != -1)
    {
      if (SYSTEM_CALL_FAILED(kill(pid, SIGKILL)))
      {
        cerr << "smash error: kill failed" << endl;
        this->jobs.erase(pid);
      }
    }
  }
}
#endif//elif temporairly_disable_kill_all_jobs

void JobsList::removeFinishedJobs()
{
  for (const auto& job: jobs)
  {
    if (waitpid(job.second.getJobPID(), nullptr, check_if_process_finished_without_blocking) != 0)
    {
      jobs.erase(job.first);
    }
  }
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  auto it = this->jobs.find(jobId);
  if (it != this->jobs.end())
  {
    return &(it->second);
  }
  return nullptr;
}

void JobsList::removeJobById(int jobId)
{
  //TODO: decide if we want to print an error if the job doesnt exist
  this->jobs.erase(jobId); // to decide, do we want the destructor of command to be called or not?
}
/*
JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
  // TODO: get last job
  return nullptr;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
  // TODO: get last stopped job
  return nullptr;
} */

int JobsList::get_max_current_jobID()
{
  Jobs &jbs = this->jobs;
  if (jbs.empty())
  {
    return 0;
  }
  else
  {
    int max_key = jbs.rbegin()->first;
    return max_key;
  }
}

int JobsList::numberOfJobs()
{
  return this->jobs.size();
}

pid_t JobsList::JobEntry::getJobPID() const
{
  return this->command->getPID();
}

void JobsList::sendSignalToJobById(int jobIDtoSendTo, int signalToSend)
{
  pid_t pid = this->getJobPID(jobIDtoSendTo);
  if (pid == -1) {
    cerr << "JobsList::sendSignalToJobById: PID not found" << endl;
    FOR_DEBUG_MODE(cerr << "Looked for pid: " << jobIDtoSendTo << endl;)
  }
  if(SYSTEM_CALL_FAILED(kill(pid, signalToSend))) {
    cerr << "JobsList::sendSignalToJobById: kill failed" << endl;
  }
}

pid_t JobsList::getJobPID(int jobID)
{
  auto it = this->jobs.find(jobID); // search for the key
  if (it != jobs.end())
  { // found the key
    return it->second.getJobPID();
  }
  else
  {            // did not find the key
    return -1; // job with jobID does not exist currently
  }
}