#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "BMDeathWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * @brief Define the UBMDeathWidget class
 * @param UBMDeathWidget The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UBMDeathWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
    // Death hint text binding
	UPROPERTY(meta=(BindWidget)) UTextBlock* DeathHintText = nullptr;
    // Restart button binding
	UPROPERTY(meta=(BindWidget)) UButton* RestartButton = nullptr;
    // Quit button binding
	UPROPERTY(meta=(BindWidget)) UButton* QuitButton = nullptr;

protected:
    // Native construct
	virtual void NativeConstruct() override;
    // Native destruct
	virtual void NativeDestruct() override;

private:
    // On restart clicked
	UFUNCTION() void OnRestartClicked();
    // On quit clicked
	UFUNCTION() void OnQuitClicked();
};
