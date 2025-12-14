#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/BMTypes.h"
#include "BMHealthBarComponent.generated.h"

class ABMCharacterBase;
class UBMStatsComponent;
class UTextRenderComponent;

/**
 * 运行时悬浮血条基类，负责与角色数值组件交互并驱动具体的可视化实现。
 *
 * 该组件只处理：
 * - 订阅角色受击/死亡事件
 * - 读取当前 HP / MaxHP
 * - 提供朝向相机的通用逻辑
 *
 * 具体展示（文本/材质/Widget）由派生类实现。
 */
UCLASS(Abstract, ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMHealthBarComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UBMHealthBarComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     * 主动指定需要监听的角色，默认会绑定到 Owner。
     */
    void ObserveCharacter(ABMCharacterBase* InCharacter);

    /**
     * 调整血条的垂直偏移高度，单位为 cm。
     */
    void SetVerticalOffset(float NewOffset);

protected:
    virtual void UpdateHealthDisplay(float CurrentHP, float MaxHP) PURE_VIRTUAL(UBMHealthBarComponent::UpdateHealthDisplay, );

    void RefreshFromStats();

private:
    void BindToCharacter(ABMCharacterBase* Character);
    void UnbindFromCharacter();
    void HandleCharacterDamaged(ABMCharacterBase* Victim, const FBMDamageInfo& Info);
    void HandleCharacterDied(ABMCharacterBase* Victim, const FBMDamageInfo& Info);
    void FaceCamera();
    void ApplyVerticalOffset();

protected:
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar")
    float VerticalOffset = 130.f;

    UPROPERTY(EditAnywhere, Category = "BM|HealthBar")
    bool bHideWhenFull = false;

    UPROPERTY(EditAnywhere, Category = "BM|HealthBar")
    bool bFaceCamera = true;

    TWeakObjectPtr<ABMCharacterBase> ObservedCharacter;
    TWeakObjectPtr<UBMStatsComponent> ObservedStats;

private:
    FDelegateHandle DamagedHandle;
    FDelegateHandle DeathHandle;
};

/**
 * 纯 C++ 文本血条实现，使用 TextRender 展示 ASCII 进度。
 */
UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMEnemyHealthBarComponent : public UBMHealthBarComponent
{
    GENERATED_BODY()

public:
    UBMEnemyHealthBarComponent();

protected:
    virtual void OnRegister() override;
    virtual void UpdateHealthDisplay(float CurrentHP, float MaxHP) override;

private:
    FString BuildBarString(float CurrentHP, float MaxHP) const;

private:
    UPROPERTY(VisibleAnywhere, Category = "BM|HealthBar")
    TObjectPtr<UTextRenderComponent> TextComponent = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|HealthBar|Display", meta = (ClampMin = "4", ClampMax = "40"))
    int32 SegmentCount = 16;

    UPROPERTY(EditAnywhere, Category = "BM|HealthBar|Display", meta = (ClampMin = "8.0", ClampMax = "64.0"))
    float FontWorldSize = 24.f;
};
