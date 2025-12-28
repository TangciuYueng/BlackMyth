// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlackMyth.h"
#include "Modules/ModuleManager.h"

/*
 * @brief Implement the primary game module for the BlackMyth module
 * @param FDefaultGameModuleImpl The default game module implementation
 * @param BlackMyth The name of the game module
 * @param "BlackMyth" The name of the game module
 */
IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, BlackMyth, "BlackMyth" );

/*
 * @brief Define the log category for the BlackMyth module
 * @param LogBlackMyth The log category name
 */
DEFINE_LOG_CATEGORY(LogBlackMyth)