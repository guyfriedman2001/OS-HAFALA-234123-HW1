#include "SmallShell.h"
#if TEMPORAIRLY_DISABLE_CTRL_HANDLER
//do nothing
#else  //if TEMPORAIRLY_DISABLE_CTRL_HANDLER

using namespace std;
#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <csignal>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <string>

#if 0
string signal_to_string(int signum) {
    switch (signum) {
        case SIGHUP:   return "SIGHUP";   // Hangup
        case SIGINT:   return "SIGINT";   // Interrupt (Ctrl+C)
        case SIGQUIT:  return "SIGQUIT";  // Quit
        case SIGILL:   return "SIGILL";   // Illegal instruction
        case SIGTRAP:  return "SIGTRAP";  // Trace/breakpoint trap
        case SIGABRT:  return "SIGABRT";  // Abort
        case SIGBUS:   return "SIGBUS";   // Bus error
        case SIGFPE:   return "SIGFPE";   // Floating point exception
        case SIGKILL:  return "SIGKILL";  // Kill, unblockable
        case SIGUSR1:  return "SIGUSR1";  // User-defined signal 1
        case SIGSEGV:  return "SIGSEGV";  // Segmentation fault
        case SIGUSR2:  return "SIGUSR2";  // User-defined signal 2
        case SIGPIPE:  return "SIGPIPE";  // Broken pipe
        case SIGALRM:  return "SIGALRM";  // Alarm clock
        case SIGTERM:  return "SIGTERM";  // Termination
        case SIGSTKFLT:return "SIGSTKFLT";// Stack fault (obsolete)
        case SIGCHLD:  return "SIGCHLD";  // Child stopped or terminated
        case SIGCONT:  return "SIGCONT";  // Continue
        case SIGSTOP:  return "SIGSTOP";  // Stop (unblockable)
        case SIGTSTP:  return "SIGTSTP";  // Keyboard stop (Ctrl+Z)
        case SIGTTIN:  return "SIGTTIN";  // Background read from tty
        case SIGTTOU:  return "SIGTTOU";  // Background write to tty
        case SIGURG:   return "SIGURG";   // Urgent condition on socket
        case SIGXCPU:  return "SIGXCPU";  // CPU limit exceeded
        case SIGXFSZ:  return "SIGXFSZ";  // File size limit exceeded
        case SIGVTALRM:return "SIGVTALRM";// Virtual alarm clock
        case SIGPROF:  return "SIGPROF";  // Profiling timer expired
        case SIGWINCH: return "SIGWINCH"; // Window resize
        case SIGIO:    return "SIGIO";    // I/O now possible
        case SIGPWR:   return "SIGPWR";   // Power failure
        case SIGSYS:   return "SIGSYS";   // Bad system call
        default:       return "Unknown Signal";
    }
}
#endif

#if NEED_TO_HANDLE_ALL_SIGNALS
string signal_to_string(int signum);
void all_signal_handler_default_wrapper(int signum);
#else // NEED_TO_HANDLE_ALL_SIGNALS
// ctrlCHandler should always be defined, and is always defined
#endif // NEED_TO_HANDLE_ALL_SIGNALS

void ctrlCHandler(int sig_num);

void setup_signal_handlers() {
#if NEED_TO_HANDLE_ALL_SIGNALS
    //only handle other signals depending on the switch blyat

    struct sigaction sa{};
    sa.sa_handler = all_signal_handler_default_wrapper;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    for (int signum = 1; signum < NSIG; ++signum) {
        // Skip signals that cannot be caught
        if (signum == SIGKILL || signum == SIGSTOP)
            continue;

        sigaction(signum, &sa, nullptr);
    }


#endif // NEED_TO_HANDLE_ALL_SIGNALS
    //always handle ctrl c
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
}


void ctrlCHandler(int sig_num)
{
    SHELL_INSTANCE.temporairly_suspend_redirection_and_return_to_default();
    printf("smash: got ctrl-C\n");
    //pid_t foreground_task = SHELL_INSTANCE.get_foreground_pid();
    if (!SHELL_INSTANCE.has_foreground_process()){//foreground_task == ERR_ARG) {
        SHELL_INSTANCE.return_from_temporary_suspension_to_what_was_changed();
        return;
    }
    //TRY_SYS(kill(foreground_task, SIGKILL), "smash error: kill failed");
    pid_t remember_what_to_print = SHELL_INSTANCE.get_foreground_pid();
    TRY_SYS2(kill(SHELL_INSTANCE.kill_foreground_process(SIGKILL), SIGKILL), "kill");
    printf("%s%d%s", SmallShell::SIGKILL_STRING_MESSAGE_1, remember_what_to_print, SmallShell::SIGKILL_STRING_MESSAGE_2);
    SHELL_INSTANCE.return_from_temporary_suspension_to_what_was_changed();
}

#if NEED_TO_HANDLE_ALL_SIGNALS
void all_signal_handler_default_wrapper(int signum) {
    SmallShell& shell = SmallShell::getInstance();

    shell.temporairly_suspend_redirection_and_return_to_default();

    if (shell.has_foreground_process()) {
        /*
        pid_t pid = shell.foreground_pid;
        kill(pid, signum);
        */

        TRY_SYS2(kill(SHELL_INSTANCE.kill_foreground_process(SIGKILL), SIGKILL), "kill");

        PRINT_DEBUG_MODE("signal " << signum << " (aka " << (strsignal(signum) ? strsignal(signum) : "unknown") << ") "<< "sent to " << shell.get_foreground_pid());

    } else {
        // Revert to default and raise again
        struct sigaction sa{};
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(signum, &sa, nullptr);
        raise(signum);
        setup_signal_handlers(); // restore behavior for next signall
    }

    shell.return_from_temporary_suspension_to_what_was_changed();
}
#else // NEED_TO_HANDLE_ALL_SIGNALS
// ctrlCHandler should always be defined, and is always defined
#endif // NEED_TO_HANDLE_ALL_SIGNALS





#define dont_need_it_at_the_end  true
#if !dont_need_it_at_the_end
void ctrl_c_handler(int signum) {
    SmallShell& shell = SmallShell::getInstance();

    shell.temporairly_suspend_redirection_and_return_to_default();

    if (shell.has_foreground_process()) {

        pid_t pid = shell.get_foreground_pid();
        shell.kill_foreground_process(signum);

        const char* name = strsignal(signum);
        std::cerr << "CTRL-C: signal " << signum
                  << " (aka " << (name ? name : "unknown") << ") "
                  << "sent to " << pid << std::endl;
    } else {
        std::cerr << "CTRL-C pressed, but no foreground process to send to" << std::endl;
    }

    shell.return_from_temporary_suspension_to_what_was_changed();
}





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
#endif //dont_need_it_at_the_end











#endif  //else TEMPORAIRLY_DISABLE_CTRL_HANDLER

