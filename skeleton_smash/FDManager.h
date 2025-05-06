//
// Created by Guy Friedman on 06/05/2025.
//

#ifndef FDMANAGER_H
#define FDMANAGER_H




class FdManager {
public:
    static FdManager& get(int target_fd);

    void redirect_to(int new_fd);
    void restore();
    void reset();

    int get_original_fd() const;

private:
    enum current_switch {
        output,
        input,
        pipe_,
        no_switch
    };

    FdManager();

    ~FdManager() = default;

    /*
    int target_fd_;
    int original_fd_;
    bool redirected_;
    */

    //TODO: add members
    bool isTempChanged;

    //explicit FdManager(int target_fd);
    FdManager(const FdManager&) = delete;
    FdManager& operator=(const FdManager&) = delete;

    void applyRedirection(); //TODO
    void undoRedirection(); //TODO
    void temp_applyRedirection(); //TODO
    void temp_undoRedirection(); //TODO
};



#endif //FDMANAGER_H
