#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
//#include "signals.h" //moved to #if && #endif code block
#include "SmallShellHeaders.h"
#include "SmallShell.h"


#define SHELL_INSTANCE   SmallShell::getInstance()

#if TEMPORAIRLY_DISABLE_CTRL_HANDLER

#if 0
void logic_before_default(int signum);
void apply_default(int signum);
void logic_after_default(int signum);
void custom_signal_handler(int signum);
#endif

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
TRY_SYS2(kill(SHELL_INSTANCE.kill_foreground_process(SIGKILL), SIGKILL), "kill");
printf("%s%d%s", SmallShell::SIGKILL_STRING_MESSAGE_1, SHELL_INSTANCE.get_foreground_pid(), SmallShell::SIGKILL_STRING_MESSAGE_2);
SHELL_INSTANCE.return_from_temporary_suspension_to_what_was_changed();
}
#else  //if TEMPORAIRLY_DISABLE_CTRL_HANDLER
#include "signals.h"
#endif  //elif TEMPORAIRLY_DISABLE_CTRL_HANDLER


#if 0
#include <csignal>
#include <iostream>
#include <unistd.h>

// === Custom logic functions ===

void logic_before_default(int signum) { //
    std::cout << "[Before] Caught signal: " << signum << std::endl;
}

void apply_default(int signum) {
    std::cout << "[Apply Default] Reverting to default handler..." << std::endl;

    struct sigaction sa{};
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(signum, &sa, nullptr) == -1) {
        perror("sigaction");
        return;
    }

    raise(signum);  // Re-raise to invoke default handling
}

void logic_after_default(int signum) {
    std::cout << "[After] Signal " << signum << " was handled.\n";
    // This will rarely run unless default handler does not terminate process
}

// === Signal handler ===

void custom_signal_handler(int signum) {
    logic_before_default(signum);
    apply_default(signum);
    logic_after_default(signum);  // Usually not reached
}

int main() {
    struct sigaction sa{};
    sa.sa_handler = custom_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGUSR1, &sa, nullptr);

    std::cout << "Send SIGUSR1 to this process (PID: " << getpid() << ")\n";
    while (true) {
        pause();  // Wait for signal
    }

    return 0;
}
#endif


int main(int argc, char *argv[]) {
    #if TEMPORAIRLY_DISABLE_CTRL_HANDLER
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    #else // TEMPORAIRLY_DISABLE_CTRL_HANDLER
    FOR_DEBUG_MODE(printf("(FOR_DEBUG_MODE) setting up cntrl handler\n");)
    setup_signal_handlers();
    #endif // TEMPORAIRLY_DISABLE_CTRL_HANDLER


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