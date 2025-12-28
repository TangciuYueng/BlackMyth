#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BMGameModeBase.generated.h"

/**
 * @brief Define the ABMGameModeBase class
 * @param ABMGameModeBase The name of the class
 * @param AGameModeBase The parent class
 */
UCLASS()
class BLACKMYTH_API ABMGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
    // Begin play
    virtual void BeginPlay() override;
public:
    // Constructor
    ABMGameModeBase();
};
