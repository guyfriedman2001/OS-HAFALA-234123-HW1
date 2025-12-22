cmake_minimum_required(VERSION 3.10.2)
project(skeleton_smash)

set(CMAKE_CXX_STANDARD 14)

add_executable(skeleton_smash smash.cpp Commands.cpp signals.cpp
        signals.h
        BuiltInCommands.h
        BuiltInCommands.cpp
        SmallShellHeaders.h
        ExternalCommands.h
        ExternalCommands.cpp
        SpecialCommands.h
        CommandFactories.h
        CommandFactories.cpp
        JobList.h
        JobList.cpp
        AliasManager.cpp
        SmallShell.h
        SmallShell.cpp
        SpecialCommands.cpp
        SmashUtil.h
        FDManager.h
        FDManager.cpp
        FDManager.cpp
        FDManager.cpp
        Temp_Restore.h
        Temp_Restore.cpp)