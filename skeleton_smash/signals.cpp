#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include "SmallShell.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    pid_t foreground_task;
    switch (sig_num)
    {
    case SIGHUP:
        //code
        break;
    case SIGINT: {
        printf("smash: got ctrl-C\n");
        foreground_task = SHELL_INSTANCE.get_foreground_pid();
        if (foreground_task == ERR_ARG) {
            return;
        }
        TRY_SYS(kill(foreground_task, SIGKILL), "smash error: kill failed");
        printf("%s%d%s", SmallShell::SIGKILL_STRING_MESSAGE_1, foreground_task, SmallShell::SIGKILL_STRING_MESSAGE_2);
        break;
    }
    case SIGQUIT:
        //code
        break;
    case SIGILL:
        //code
        break;
    case SIGTRAP:
        //code
        break;
    case SIGABRT:
        //code
        break;
    case SIGBUS:
        //code
        break;
    case SIGFPE:
        //code
        break;
    case SIGKILL:
        //code
        break;
    case SIGUSR1:
        //code
        break;
    case SIGSEGV:
        //code
        break;
    case SIGUSR2:
        //code
        break;
    case SIGPIPE:
        //code
        break;
    case SIGALRM:
        //code
        break;
    case SIGTERM:
        //code
        break;
    case SIGCHLD:
        //code
        break;
    case SIGCONT:
        //code
        break;
    case SIGSTOP:
        //code
        break;
    case SIGTSTP:
        //code
        break;
    case SIGTTIN:
        //code
        break;
    case SIGTTOU:
        //code
        break;
    case SIGURG:
        //code
        break;
    case SIGXCPU:
        //code
        break;
    case SIGXFSZ:
        //code
        break;
    case SIGVTALRM:
        //code
        break;
    case SIGPROF:
        //code
        break;
    case SIGWINCH:
        //code
        break;
    case SIGIO:
        //code
        break;
    //case SIGPWR:
        //code
        //break;
    case SIGSYS:
        //code
        break;
    
    default:
        break;
    }

}
