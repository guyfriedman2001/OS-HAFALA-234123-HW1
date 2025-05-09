//
// Created by Guy Friedman on 06/05/2025.
//
#if 0
#include "Temp_Restore.h"
#include "FDManager.h"

Temp_Restore::Temp_Restore()
{
    FDManager::getFDManager().temp_undoRedirection();
}

Temp_Restore::~Temp_Restore()
{
    FDManager::getFDManager().temp_returnToRedirection();
}
#endif