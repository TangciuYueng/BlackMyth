#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "Core/BMTypes.h"
class UButton;
class UTextBlock;
#include "BMSaveLoadMenuWidget.generated.h"

/**
 * @brief Define the UBMSaveLoadMenuWidget class, save and load menu widget
 * @param UBMSaveLoadMenuWidget The name of the class
 * @param UBMWidgetBase The parent class
 */
UCLASS()
class BLACKMYTH_API UBMSaveLoadMenuWidget : public UBMWidgetBase
{
    GENERATED_BODY()
public:
    // Back button binding
    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    // Slot 1 button binding
    UPROPERTY(meta = (BindWidget))
    UButton* Slot1;

protected:
    // Native construct
    virtual void NativeConstruct() override;
    // Native destruct
    virtual void NativeDestruct() override;

private:
    // 按钮点击事件处理函数
    UFUNCTION()
    void OnBackClicked();

    // On slot 1 clicked
    UFUNCTION()
    void OnSlot1Clicked();

    // On slot 2 clicked
    UFUNCTION()
    void OnSlot2Clicked();

    // On slot 3 clicked
    UFUNCTION()
    void OnSlot3Clicked();

    // On slot 4 clicked
    UFUNCTION()
    void OnSlot4Clicked();

    // 执行加载逻辑
    void LoadGameIndex(int32 SlotIndex);

    // 刷新存档槽位显示
    void RefreshSaveSlotDisplay();

    // 更新单个槽位按钮显示
    void UpdateSlotButton(UButton* Button, int32 SlotIndex, const TArray<FBMSaveSlotInfo>& SaveSlots);

    // 设置按钮文本（通过查找子TextBlock）
    void SetButtonText(UButton* Button, const FText& Text);
};