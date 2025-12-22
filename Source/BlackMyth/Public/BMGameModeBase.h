#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BMGameModeBase.generated.h"

class USoundBase;

UCLASS()
class BLACKMYTH_API ABMGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
    virtual void BeginPlay() override;
public:
    ABMGameModeBase();

protected:
	UPROPERTY(VisibleAnywhere, Category = "BM|Audio")
	TObjectPtr<USoundBase> Level1StartMusic = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BM|Audio")
	TObjectPtr<USoundBase> Level2StartMusic = nullptr;
};
