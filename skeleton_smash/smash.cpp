#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include "SmallShell.h"

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