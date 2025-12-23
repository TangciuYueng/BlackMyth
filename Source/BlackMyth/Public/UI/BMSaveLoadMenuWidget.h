#pragma once

#include "CoreMinimal.h"
#include "UI/BMWidgetBase.h"
#include "Core/BMTypes.h"
class UButton;
class UTextBlock;
#include "BMSaveLoadMenuWidget.generated.h"

UCLASS()
class BLACKMYTH_API UBMSaveLoadMenuWidget : public UBMWidgetBase
{
    GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;    // 对应截图中的 BackButton

    // 假设你的 Slot1 - Slot4 是普通的按钮 (Button)
    // 如果它们是你自己做的“用户控件”，这里需要改成你的自定义类指针
    UPROPERTY(meta = (BindWidget))
    UButton* Slot1;

    UPROPERTY(meta = (BindWidget))
    UButton* Slot2;

    UPROPERTY(meta = (BindWidget))
    UButton* Slot3;

    UPROPERTY(meta = (BindWidget))
    UButton* Slot4;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    // 按钮点击事件处理函数
    UFUNCTION()
    void OnBackClicked();

    UFUNCTION()
    void OnSlot1Clicked();

    UFUNCTION()
    void OnSlot2Clicked();

    UFUNCTION()
    void OnSlot3Clicked();

    UFUNCTION()
    void OnSlot4Clicked();

    // 辅助函数：执行加载逻辑
    void LoadGameIndex(int32 SlotIndex);

    // 刷新存档槽位显示
    void RefreshSaveSlotDisplay();

    // 更新单个槽位按钮显示
    void UpdateSlotButton(UButton* Button, int32 SlotIndex, const TArray<FBMSaveSlotInfo>& SaveSlots);

    // 设置按钮文本（通过查找子TextBlock）
    void SetButtonText(UButton* Button, const FText& Text);
};