cmake_minimum_required(VERSION 3.19)
project(skeleton_smash)

set(CMAKE_CXX_STANDARD 14)

add_executable(skeleton_smash smash.cpp Commands.cpp signals.cpp
        signals.h
        main.cpp
        BuiltInCommands.h
        BuiltInCommands.cpp
        SmallShellHeaders.h
        ExternalCommands.h
        ExternalCommands.cpp
        ComplexExternalCommands.cpp
        SpecialCommands.h
        CommandFactories.h
        CommandFactories.cpp
        JobList.h
        JobList.cpp
        AliasManager.cpp
        SmallShell.h
        SmallShell.cpp)