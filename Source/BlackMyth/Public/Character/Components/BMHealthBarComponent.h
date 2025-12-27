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
 * 该组件处理：
 * - 订阅角色受击/死亡事件
 * - 读取当前 HP / MaxHP
 * - 提供朝向相机的通用逻辑
 */
UCLASS(Abstract, ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMHealthBarComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UBMHealthBarComponent();

    /**
     * 组件开始运行
     *
     * 应用垂直偏移并自动监听 Owner 角色（若未手动指定）
     */
    virtual void BeginPlay() override;

    /**
     * 组件结束运行
     *
     * @param EndPlayReason 结束原因
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * 每帧更新
     *
     * 若启用朝向相机功能，每帧调整朝向以面向玩家相机
     *
     * @param DeltaTime 帧时间间隔
     * @param TickType Tick 类型
     * @param ThisTickFunction Tick 函数指针
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /**
     * 指定要监听的角色
     *
     * 切换监听目标，解绑旧角色事件并绑定到新角色的受击/死亡事件
     * 若不调用，默认监听 Owner 角色
     *
     * @param InCharacter 要监听的目标角色
     */
    void ObserveCharacter(ABMCharacterBase* InCharacter);

    /**
     * 调整血条的垂直偏移高度
     *
     * @param NewOffset 新的偏移值（单位：cm），相对于角色根位置
     */
    void SetVerticalOffset(float NewOffset);

protected:
    /**
     * 更新血条显示
     *
     * @param CurrentHP 当前生命值
     * @param MaxHP 最大生命值
     */
    virtual void UpdateHealthDisplay(float CurrentHP, float MaxHP) PURE_VIRTUAL(UBMHealthBarComponent::UpdateHealthDisplay, );

    /**
     * 从 Stats 组件刷新血条显示
     *
     * 读取监听角色的当前 HP/MaxHP 并调用 UpdateHealthDisplay
     */
    void RefreshFromStats();

private:
    /**
     * 绑定到角色事件
     *
     * @param Character 目标角色
     */
    void BindToCharacter(ABMCharacterBase* Character);

    /**
     * 解绑当前角色事件
     */
    void UnbindFromCharacter();

    /**
     * 处理角色受击事件
     *
     * @param Victim 受害者
     * @param Info 伤害信息
     */
    void HandleCharacterDamaged(ABMCharacterBase* Victim, const FBMDamageInfo& Info);

    /**
     * 处理角色死亡事件
     *
     * @param Victim 死亡角色
     * @param Info 伤害信息
     */
    void HandleCharacterDied(ABMCharacterBase* Victim, const FBMDamageInfo& Info);

    /**
     * 让血条朝向相机
     *
     * 每帧调整血条朝向以面向玩家相机
     */
    void FaceCamera();

    /**
     * 应用垂直偏移
     *
     * 根据 VerticalOffset 设置组件的相对位置
     */
    void ApplyVerticalOffset();

protected:
    /** 血条垂直偏移高度（相对于角色根位置） */
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar")
    float VerticalOffset = 130.f;

    /** 血量满时是否隐藏血条 */
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar")
    bool bHideWhenFull = false;

    /** 是否每帧朝向相机 */
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar")
    bool bFaceCamera = true;

    /** 当前监听的角色 */
    TWeakObjectPtr<ABMCharacterBase> ObservedCharacter;

    /** 当前监听的数值组件 */
    TWeakObjectPtr<UBMStatsComponent> ObservedStats;

private:
    /** 受击事件绑定句柄 */
    FDelegateHandle DamagedHandle;

    /** 死亡事件绑定句柄 */
    FDelegateHandle DeathHandle;
};

/**
 * 敌人文本血条实现
 *
 * 使用双层 TextRender 展示彩色进度条：
 * - 背景层（灰色）：显示完整的空血条
 * - 前景层（红色）：显示当前血量
 */
UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMEnemyHealthBarComponent : public UBMHealthBarComponent
{
    GENERATED_BODY()

public:
    UBMEnemyHealthBarComponent();

protected:
    /**
     * 组件注册
     *
     * 创建并配置背景和前景文本组件
     */
    virtual void OnRegister() override;

    /**
     * 更新血条显示
     *
     * 根据当前 HP 百分比计算填充段数并更新前景文本
     *
     * @param CurrentHP 当前生命值
     * @param MaxHP 最大生命值
     */
    virtual void UpdateHealthDisplay(float CurrentHP, float MaxHP) override;

private:
    /**
     * 构建血量前景字符串
     *
     * @param FilledCount 填充的段数
     * @return 前景血条字符串
     */
    FString BuildFilledBarString(int32 FilledCount) const;

    /**
     * 构建背景字符串
     *
     * @return 完整的背景血条字符串
     */
    FString BuildBackgroundBarString() const;

private:
    /** 显示完整的空血条 */
    UPROPERTY(VisibleAnywhere, Category = "BM|HealthBar")
    TObjectPtr<UTextRenderComponent> BackgroundTextComponent = nullptr;

    /** 显示当前血量 */
    UPROPERTY(VisibleAnywhere, Category = "BM|HealthBar")
    TObjectPtr<UTextRenderComponent> ForegroundTextComponent = nullptr;

    /** 血条分段数量 */
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar|Display", meta = (ClampMin = "4", ClampMax = "40"))
    int32 SegmentCount = 30;

    /** 字体世界大小 */
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar|Display", meta = (ClampMin = "8.0", ClampMax = "64.0"))
    float FontWorldSize = 20.f;

    /** 血量颜色（红色） */
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar|Display")
    FColor HealthColor = FColor(220, 20, 20, 255);

    /** 背景颜色（灰色） */
    UPROPERTY(EditAnywhere, Category = "BM|HealthBar|Display")
    FColor BackgroundColor = FColor(80, 80, 80, 255);
};
