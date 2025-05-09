#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
//#include "signals.h" //moved to #if && #endif code block
#include "SmallShellHeaders.h"
#include "SmallShell.h"


#define SHELL_INSTANCE   SmallShell::getInstance()




void ctrlCHandler(int sig_num)
{


    printf("smash: got ctrl-C\n");
    //pid_t foreground_task = SHELL_INSTANCE.get_foreground_pid();
    if (!SHELL_INSTANCE.has_foreground_process()){//foreground_task == ERR_ARG) {
        //printf("smash> ");
        //SHELL_INSTANCE.printPrompt();
        //cout.flush();
        return;
    }
    //TRY_SYS(kill(foreground_task, SIGKILL), "smash error: kill failed");
    pid_t remember_what_to_print = SHELL_INSTANCE.get_foreground_pid();
     if (waitpid(remember_what_to_print, nullptr, WNOHANG) != 0) {
        //SHELL_INSTANCE.printPrompt();
        //cout.flush();
        return;
    }
    TRY_SYS2(kill(remember_what_to_print, SIGKILL), "kill");
    //SHELL_INSTANCE.getJobsList().removeFinishedJobs();
    printf("%s%d%s", SmallShell::SIGKILL_STRING_MESSAGE_1, remember_what_to_print, SmallShell::SIGKILL_STRING_MESSAGE_2);

}

int main(int argc, char *argv[]) {

    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler\n");
    }


    SmallShell &smash = SHELL_INSTANCE;
    while (true) {
        //cout << smash.getPrompt() << smash.getEndStr();
        smash.printPrompt();
        string cmd_line;
        getline(cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
        //cout << endl;
    }
    return 0;
}

