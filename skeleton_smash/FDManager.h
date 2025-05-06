//
// Created by Guy Friedman on 06/05/2025.
//

#ifndef FDMANAGER_H
#define FDMANAGER_H
#include "SmallShellHeaders.h"
#include "Temp_Restore.h"
//#define class_to_manage_its_fd SmallShell

/*
static MySingleton& getInstance() {
        static MySingleton instance;  // created once, thread-safe in C++11+
        return instance;
    } */

class FdManager {
public:
    static FdManager& getFDManager() {
        static FdManager instance;
        return instance;
    }

private:
    friend SmallShell;
    void applyRedirection(const char *cmd_line, const argv &args, argv &remaining_args,fd_location &std_in,fd_location &std_out,fd_location &std_err, open_flag flag);
    void undoRedirection(const char *cmd_line, const argv &args, argv &remaining_args,fd_location &std_in,fd_location &std_out,fd_location &std_err, open_flag flag);
    void undoRedirection(const argv &args);
    void undoRedirection(const char *cmd_line);
    void undoRedirection();
    void undoSpecificRedirection(fd_location &saved_location, fd_location destination_location);
private:
    //forward declare the class
    friend Temp_Restore; //if this shit doesnt work, just move these two functions to public
    void return_from_temporary_suspension_to_what_was_changed();
    void temporairly_suspend_redirection_and_return_to_default();
private:

    //enum current_switch {output, input, pipe_, no_switch};

    static fd_location m_current_std_in;
    static fd_location m_current_std_out;
    static fd_location m_current_std_error;

    static fd_location m_extern_std_in;
    static fd_location m_extern_std_out;
    static fd_location m_extern_std_error;

    void closed_extern_channel(fd_location &closed_channel);

    FdManager();

    ~FdManager();

    //TODO: add members
    bool isTempChanged;
    bool isRedirected;

    //explicit FdManager(int target_fd);
    FdManager(const FdManager&) = delete;
    FdManager& operator=(const FdManager&) = delete;

    void switch_two_fd_entries(fd_location &entry1, fd_location &entry2);


};



#endif //FDMANAGER_H
