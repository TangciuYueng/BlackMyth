#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BMGameHUD.generated.h"

/**
 * Minimal HUD actor to satisfy GameMode HUDClass.
 * UMG HUD widgets are managed by UBMUIManagerSubsystem.
 */
UCLASS()
class BLACKMYTH_API ABMGameHUD : public AHUD
{
    GENERATED_BODY()
};
