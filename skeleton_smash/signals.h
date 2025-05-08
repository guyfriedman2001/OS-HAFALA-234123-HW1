#include "SmallShellHeaders.h"

#if !PERMANENTLY_DISABLE_SIGNALS_H

#ifndef SMASH__SIGNALS_H_
#define SMASH__SIGNALS_H_



#if TEMPORAIRLY_DISABLE_CTRL_HANDLER
#else  //if TEMPORAIRLY_DISABLE_CTRL_HANDLER

//void signal_handler(int signum);

void setup_signal_handlers();

//void ctrlCHandler(int sig_num);

#if 0
void ctrlCHandler(int sig_num);
#endif
#endif  //elif TEMPORAIRLY_DISABLE_CTRL_HANDLER


#endif //SMASH__SIGNALS_H_
#endif
