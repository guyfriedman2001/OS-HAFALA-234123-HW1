#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"



#define SHELL_INSTANCE   SmallShell::getInstance()

#if TEMPORAIRLY_DISABLE_CTRL_HANDLER
void ctrlCHandler(int sig_num)
{
printf("smash: got ctrl-C\n");
pid_t foreground_task = SHELL_INSTANCE.get_foreground_pid();
if (foreground_task == ERR_ARG) {
    return;
}
//TRY_SYS(kill(foreground_task, SIGKILL), "smash error: kill failed");
TRY_SYS2(kill(foreground_task, SIGKILL), "kill");
printf("%s%d%s", SmallShell::SIGKILL_STRING_MESSAGE_1, foreground_task, SmallShell::SIGKILL_STRING_MESSAGE_2);
}
#elif  //if TEMPORAIRLY_DISABLE_CTRL_HANDLER
#include "SmallShell.h"
#endif  //elif TEMPORAIRLY_DISABLE_CTRL_HANDLER



int main(int argc, char *argv[]) {
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
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