//
// Created by Guy Friedman on 06/05/2025.
//

#include "FDManager.h"

#include "SmallShell.h"

#define CLOSED_CHANNEL (-1)
#define CHANNEL_IS_CLOSED(CHANNEL) (CHANNEL == CLOSED_CHANNEL)

fd_location FdManager::m_current_std_in = STDIN_FILE_NUM;
fd_location FdManager::m_current_std_out = STDOUT_FILE_NUM;
fd_location FdManager::m_current_std_error = STDERR_FILE_NUM;
fd_location FdManager::m_extern_std_in = CLOSED_CHANNEL;
fd_location FdManager::m_extern_std_out = CLOSED_CHANNEL;
fd_location FdManager::m_extern_std_error = CLOSED_CHANNEL;

/*
m_current_std_in = STDIN_FILE_NUM;
  m_current_std_out = STDOUT_FILE_NUM;
  m_current_std_error = STDERR_FILE_NUM;

  closed_extern_channel(m_extern_std_in);
  closed_extern_channel(m_extern_std_out);
  closed_extern_channel(m_extern_std_error);
 */

bool isRedirectionCommand(const char *cmd_line)
{
  if (isIORedirectionCommand(cmd_line))
  {
    return true;
  }
  if (isPipeCommand(cmd_line))
  { //TODO: maybe we want to handle the pipe logic on a different scope? YES.
    return true;
  }
  return false;
}

open_flag getFlagSingleArg(const std::string& arg) {
  if (STRINGS_EQUAL(arg, "<")) {
    return O_RDONLY;
  } else if (STRINGS_EQUAL(arg, ">")) {
    return O_WRONLY | O_CREAT | O_TRUNC;
  } else if (STRINGS_EQUAL(arg, ">>")) {
    return O_WRONLY | O_CREAT | O_APPEND;
  } else if (STRINGS_EQUAL(arg, "<<")) {
    return O_RDONLY;
  } else { //wrong argument given
    return ERR_ARG;
  }
}

open_flag getFlagVectorArg(const argv& args) {
  open_flag flag = ERR_ARG;
  for (const auto& arg : args) {
    if(getFlagSingleArg(arg) != ERR_ARG) {
      flag = getFlagSingleArg(arg);
      break;
    }
  }
  return flag;
}

bool is_stderr_pipe(const argv& args)
{
  assert_not_empty(args);
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "|" || args[i] == "|&") {
      return (args[i] == "|&");
    }
  }
}

bool is_stderr_pipe(const char* cmd_line)
{
  //TODO
  return false;
}

int get_arg_split_idx(const argv& args, const string& compare_blyat) {
  // Find the pipe symbol
  int pipe_index = -1;
  for (int i = 0; i < args.size(); ++i) {
    if (STRINGS_EQUAL(args[i],compare_blyat)){
      pipe_index = i;
      break;
    }
  }
  return pipe_index;
}

void split_args_by_index(const argv& args, argv& left_args, argv& right_args, int split_idx)
{
  assert(split_idx != -1); // must have | or |&
  // Split left and right commands
  left_args = argv(args.begin(), args.begin() + split_idx);
  right_args = argv(args.begin() + split_idx + 1, args.end());
}

void split_output(const argv& args, argv& left_args, argv& right_args)
{
  int idx = get_arg_split_idx(args, ">");
  int idx2 = get_arg_split_idx(args, ">>");
  int actual_idx = MAX(idx,idx2);
  split_args_by_index(args, left_args, right_args, actual_idx);
}

void split_input(const argv& args, argv& left_args, argv& right_args)
{
  int idx = get_arg_split_idx(args, "<");
  int idx2 = get_arg_split_idx(args, "<<");
  int actual_idx = MAX(idx,idx2);
  split_args_by_index(args, left_args, right_args, actual_idx);
}

void split_pipe(const argv& args, argv& left_args, argv& right_args)
{
  int idx = get_arg_split_idx(args, "|");
  int idx2 = get_arg_split_idx(args, "|&");
  int actual_idx = MAX(idx,idx2);
  split_args_by_index(args, left_args, right_args, actual_idx);
}

void create_pipe(const argv& args, argv& left_args, argv& right_args,fd_location &std_in,
                  fd_location &std_out,fd_location &std_err, bool isCerrPipe) //TODO BALAT needs testing blyat
{ //TODO: if pupe changes daddys fd, then need to update m_extern to be on their opened destinations
  SmallShell &SHELL_INSTANCE = SmallShell::getInstance();
  pid_t first_born = fork();
  TRY_SYS2(first_born,"fork");
  if (first_born != 0){return;} //if im the daddy

  //now im working as the first born (ya'ani ben bechor), this part will use @left_args
  //start by making a pipe
  BIBE my_pipe[BIBE_SIZE];
  TRY_SYS2(pipe(my_pipe), "pipe");

  //now after i made my pipe, i want to make myself a son to read my yapping
  pid_t second_born = fork();
  TRY_SYS2(second_born,"fork");

  if (second_born != 0) //todo: decide if i want to make a 'void SmallShell::executeCommand(const argv& args)' wrapper for 'void SmallShell::executeCommand(const char *cmd_line)', or it may cause broblems? YES, YHIHE BESEDER
  { //meaning i am the firstborn

    //close unused bibe end
    TRY_SYS2(close(my_pipe[BIBE_READ]),"close");

    //use given bool to decide if i am changing my cout or cerr into the bibe
    fd_location bibe_outbut_target = ((isCerrPipe) ? STDERR_FILE_NUM : STDOUT_FILE_NUM);

    //migrate used bibe end to stdout/stderr depending on given bool
    TRY_SYS2(dup2(my_pipe[BIBE_READ],bibe_outbut_target),"dup2");

    //close redundant bibe end
    TRY_SYS2(close(my_pipe[BIBE_WRITE]),"close");

    //use existing smash logic for the rest of the process
    SHELL_INSTANCE.executeCommand(left_args);

    //after process finished no longer need for firsborn
    exit(0);

  } else { //meaning i am the second born (yaani neched), this part would take @right_args

    //close unused bibe end
    TRY_SYS2(close(my_pipe[BIBE_WRITE]),"close");

    //migrate used bibe end to std in
    TRY_SYS2(dup2(my_pipe[BIBE_READ],STDIN_FILE_NUM),"dup2");

    //closed redundant bibe end
    TRY_SYS2(close(my_pipe[BIBE_READ]),"close");

    //use existing smash logic for the rest of the process
    SHELL_INSTANCE.executeCommand(right_args);

    //after process finished no longer need for secondborn
    exit(0);
  }
}

FdManager::FdManager()
{
  m_current_std_in = STDIN_FILE_NUM;
  m_current_std_out = STDOUT_FILE_NUM;
  m_current_std_error = STDERR_FILE_NUM;

  closed_extern_channel(m_extern_std_in);
  closed_extern_channel(m_extern_std_out);
  closed_extern_channel(m_extern_std_error);
}

FdManager::~FdManager()
{
  undoRedirection();
}


void FdManager::undoSpecificRedirection(fd_location &saved_location, fd_location destination_location)
{
  if (destination_location == saved_location) {
    return;
  }
  int success1 = dup2(saved_location, destination_location);
  TRY_SYS2(success1,"dup2");
  if (SYSTEM_CALL_FAILED(success1)){return;}
  int success2 = close(saved_location);
  TRY_SYS2(success2,"close");
  if (!SYSTEM_CALL_FAILED(success2)) {
    saved_location = destination_location;
  }
}



void FdManager::applyRedirection(const char *cmd_line, const argv &args, argv &remaining_args,fd_location &std_in,fd_location &std_out,fd_location &std_err, open_flag flag)
{
#if 0
  assert(isRedirectionCommand(cmd_line));
  open_flag flag = getFlagVectorArg(args);
  argv left_arguments, right_arguments;
  if (isInputRedirectionCommand(cmd_line)) {
    assert(isInputRedirectionCommand(cmd_line) && "Expected input redirection command!");
    split_input(args, left_arguments, right_arguments);
    std_in = dup(STDIN_FILE_NUM);
    TRY_SYS2(std_in,"dup");

    FOR_DEBUG_MODE(
    std::cerr << "(FOR_DEBUG_MODE)  " << "Right arg: " << right_arguments[0] << std::endl;
    )


    fd_location new_temp_fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(new_temp_fd,"open");
    TRY_SYS2(dup2(new_temp_fd,STDIN_FILE_NUM),"dup2");
    TRY_SYS2(close(new_temp_fd),"close");

    /*
    TRY_SYS2(close(STDIN_FILE_NUM),"close");
    fd_location fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(fd,"open");
    assert(fd == STDIN_FILE_NUM);
    */

    //initialise remaining arguments vector
    remaining_args = left_arguments;
  } else if (isOutputRedirectionCommand(cmd_line)) {
    assert(isOutputRedirectionCommand(cmd_line) && "Expected output redirection command!");
    split_output(args, left_arguments, right_arguments);
    std_out = dup(STDOUT_FILE_NUM);
    TRY_SYS2(std_out,"dup");

    FOR_DEBUG_MODE(
    std::cerr << "(FOR_DEBUG_MODE)  " << "Right arg: " << right_arguments[0] << std::endl;
    )


    fd_location new_temp_fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(new_temp_fd,"open");
    TRY_SYS2(dup2(new_temp_fd,STDOUT_FILE_NUM),"dup2");
    TRY_SYS2(close(new_temp_fd),"close");

    /*
    TRY_SYS2(close(STDOUT_FILE_NUM),"close");
    fd_location fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(fd,"open");
    assert(fd == STDOUT_FILE_NUM);
    */

    //initialise remaining arguments vector
    remaining_args = left_arguments;
  } else if (isPipeCommand(cmd_line)) {
    split_pipe(args, left_arguments, right_arguments);
    create_pipe(args,left_arguments,right_arguments,std_in,std_out,std_err,is_stderr_pipe(args));
    //TODO: IF WE ARE IN PIPE COMMAND, NEED TO INITIALISE remaining_args IN A WAY THAT WOULD SIGNALL THE SYSTEM TO STOP WITH THE NEXT COMMAND
    //OR IF PIPE SHOULD BE IN FOREGROUND, THEN NEED TO SPLIT ARGUMENTS AND APPLY TO REMAINING ARGS, LIKE IN THE NEXT COMMENTED LINE
  } else {
    FOR_DEBUG_MODE(
    perror("unknown redirection command in 'void applyRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)'\n");
    )
  }
  //THE NEXT COMMENTED LINE:
  //remaining_args = left_arguments;
#else
  applyRedirection(cmd_line, args, remaining_args);
#endif
}

void FdManager::applyRedirection(const char *cmd_line, const argv &args, argv &remaining_args) //this is the new one
{
  assert(isRedirectionCommand(cmd_line));
  open_flag flag = getFlagVectorArg(args);
  argv left_arguments, right_arguments;
  if (isInputRedirectionCommand(cmd_line)) {
    assert(isInputRedirectionCommand(cmd_line) && "Expected input redirection command!");
    split_input(args, left_arguments, right_arguments);
    m_current_std_in = dup(STDIN_FILE_NUM);
    TRY_SYS2(m_current_std_in,"dup");

    FOR_DEBUG_MODE(
    std::cerr << "(FOR_DEBUG_MODE)  " << "Right arg: " << right_arguments[0] << std::endl;
    )


    fd_location new_temp_fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(new_temp_fd,"open");
    TRY_SYS2(dup2(new_temp_fd,STDIN_FILE_NUM),"dup2");
    TRY_SYS2(close(new_temp_fd),"close");

    /*
    TRY_SYS2(close(STDIN_FILE_NUM),"close");
    fd_location fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(fd,"open");
    assert(fd == STDIN_FILE_NUM);
    */

    //initialise remaining arguments vector
    remaining_args = left_arguments;
  } else if (isOutputRedirectionCommand(cmd_line)) {
    assert(isOutputRedirectionCommand(cmd_line) && "Expected output redirection command!");
    split_output(args, left_arguments, right_arguments);
    m_current_std_out = dup(STDOUT_FILE_NUM);
    TRY_SYS2(m_current_std_out,"dup");

    FOR_DEBUG_MODE(
    std::cerr << "(FOR_DEBUG_MODE)  " << "Right arg: " << right_arguments[0] << std::endl;
    )


    fd_location new_temp_fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(new_temp_fd,"open");
    TRY_SYS2(dup2(new_temp_fd,STDOUT_FILE_NUM),"dup2");
    TRY_SYS2(close(new_temp_fd),"close");

    /*
    TRY_SYS2(close(STDOUT_FILE_NUM),"close");
    fd_location fd = open(right_arguments[0].c_str(), flag, OPEN_IN_GOD_MODE);
    TRY_SYS2(fd,"open");
    assert(fd == STDOUT_FILE_NUM);
    */

    //initialise remaining arguments vector
    remaining_args = left_arguments;
  } else if (isPipeCommand(cmd_line)) {
    split_pipe(args, left_arguments, right_arguments);
    create_pipe(args,left_arguments,right_arguments,m_current_std_in,m_current_std_out,m_extern_std_error,is_stderr_pipe(args));
    //TODO: IF WE ARE IN PIPE COMMAND, NEED TO INITIALISE remaining_args IN A WAY THAT WOULD SIGNALL THE SYSTEM TO STOP WITH THE NEXT COMMAND
    //OR IF PIPE SHOULD BE IN FOREGROUND, THEN NEED TO SPLIT ARGUMENTS AND APPLY TO REMAINING ARGS, LIKE IN THE NEXT COMMENTED LINE
  } else {
    FOR_DEBUG_MODE(
    perror("unknown redirection command in 'void applyRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)'\n");
    )
  }
  //THE NEXT COMMENTED LINE:
  //remaining_args = left_arguments;
}

void FdManager::closed_extern_channel(fd_location &closed_channel)
{
  closed_channel = CLOSED_CHANNEL;
}


void FdManager::switch_two_fd_entries(fd_location &entry1, fd_location &entry2)
{
  if (entry1 == entry2) {
    return;
  }
  if (CHANNEL_IS_CLOSED(entry1) || CHANNEL_IS_CLOSED(entry2)) {
    return;
  }
  fd_location temp = dup(entry1);
  TRY_SYS2(temp,"dup");
  if (SYSTEM_CALL_FAILED(temp)){return;}
  int move_2_to_1 = dup2(entry2,entry1);
  TRY_SYS2(move_2_to_1,"dup2");
  if (SYSTEM_CALL_FAILED(move_2_to_1)){return;}
  int move_temp_to_2 = dup2(temp,entry2);
  TRY_SYS2(move_temp_to_2,"dup2");
  if (SYSTEM_CALL_FAILED(move_temp_to_2)){return;}
  int close_temp = close(temp);
  TRY_SYS2(close_temp,"close");
}

void FdManager::return_from_temporary_suspension_to_what_was_changed()
{
  if (!isTempChanged) {return;}
  switch_two_fd_entries(m_current_std_in,m_extern_std_in);
  switch_two_fd_entries(m_current_std_out,m_extern_std_out);
  switch_two_fd_entries(m_current_std_error,m_extern_std_error);
  isTempChanged = false;
}//TODO

void FdManager::temporairly_suspend_redirection_and_return_to_default()
{
  if (isTempChanged) {return;}
  switch_two_fd_entries(m_current_std_in,m_extern_std_in);
  switch_two_fd_entries(m_current_std_out,m_extern_std_out);
  switch_two_fd_entries(m_current_std_error,m_extern_std_error);
  isTempChanged = true;
}


void FdManager::undoRedirection()
{
  return_from_temporary_suspension_to_what_was_changed();
  undoSpecificRedirection(m_current_std_in, STDIN_FILE_NUM);
  closed_extern_channel(m_extern_std_in);
  undoSpecificRedirection(m_current_std_out, STDOUT_FILE_NUM);
  closed_extern_channel(m_extern_std_out);
  undoSpecificRedirection(m_current_std_error, STDERR_FILE_NUM);
  closed_extern_channel(m_extern_std_error);
}

void FdManager::undoRedirection(const char *cmd_line){undoRedirection();}
#if 0
{
  assert(isRedirectionCommand(cmd_line));
  if (isInputRedirectionCommand(cmd_line)) {
    //close(STDIN_FILE_NUM); it appears that dup2 handles this case
    TRY_SYS2(dup2(m_current_std_in, STDIN_FILE_NUM),"dup2");
    TRY_SYS2(close(m_current_std_in),"close");
  } else if (isOutputRedirectionCommand(cmd_line)) {
    TRY_SYS2(dup2(m_current_std_out, STDOUT_FILE_NUM),"dup2");
    TRY_SYS2(close(m_current_std_out),"close");
#if PIPE_CHANGES_DADDYS_FD
  } else if (isPipeCommand(cmd_line)) {
    //revert input back to original

    if (is_stderr_pipe(cmd_line))
    {
      //revert cerr pipe back to original
      TRY_SYS2(dup2(m_current_std_error, STDERR_FILE_NUM),"dup2");
      TRY_SYS2(close(m_current_std_error),"close");
    } else {
      //revert cout pipe back to original
      TRY_SYS2(dup2(m_current_std_out, STDOUT_FILE_NUM),"dup2");
      TRY_SYS2(close(m_current_std_out),"close");
    }
#endif //if PIPE_CHANGES_DADDYS_FD
  } else {
    FOR_DEBUG_MODE(
    perror("unknown redirection command in 'void undoRedirection(const char *cmd_line, const argv &args,fd_location &std_in,fd_location &std_out,fd_location &std_err)'\n");
    )
  }
}
#endif

void FdManager::undoRedirection(const char *cmd_line, const argv &args, argv &remaining_args,fd_location &std_in,fd_location &std_out,fd_location &std_err, open_flag flag){undoRedirection();}
void FdManager::undoRedirection(const argv &args){undoRedirection();}
