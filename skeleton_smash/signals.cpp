#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
    switch (sig_num)
    {
    case SIGHUP:
        //code
        break;
    case SIGINT:
        //code
        break;
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
