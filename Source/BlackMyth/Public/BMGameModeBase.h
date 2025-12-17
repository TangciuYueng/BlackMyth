#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BMGameModeBase.generated.h"

UCLASS()
class BLACKMYTH_API ABMGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
    ABMGameModeBase();

private:
    // [TEST ONLY] Enable per-second health drain to verify HUD updates.
    // Set true in editor or via defaults to simulate HP dropping by 0.1 every second until 0.
    UPROPERTY(EditAnywhere, Category = "BM|Test")
    bool bEnableHealthDrainTest = false;

    // [TEST ONLY] Internal state for the health drain test.
    FTimerHandle HealthDrainTestTimer;
    float TestHealth01 = 1.0f;
};
