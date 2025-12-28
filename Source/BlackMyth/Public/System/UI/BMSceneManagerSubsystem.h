// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BMSceneManagerSubsystem.generated.h"

/**
 * @brief Define the UBMSceneManagerSubsystem class, manager for the scene, used to manage the scene transitions
 * @param UBMSceneManagerSubsystem The name of the class
 * @param UGameInstanceSubsystem The parent class
 */
UCLASS()
class BLACKMYTH_API UBMSceneManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
};
