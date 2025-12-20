#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "BMDeathWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class BLACKMYTH_API UBMDeathWidget : public UBMWidgetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget)) UTextBlock* DeathHintText = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton* RestartButton = nullptr;
	UPROPERTY(meta=(BindWidget)) UButton* QuitButton = nullptr;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION() void OnRestartClicked();
	UFUNCTION() void OnQuitClicked();
};
