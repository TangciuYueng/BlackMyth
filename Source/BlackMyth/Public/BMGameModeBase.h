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
public:
    ABMGameModeBase();
};
