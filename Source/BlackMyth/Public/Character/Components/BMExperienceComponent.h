#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BMExperienceComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBMExperience, Log, All);

DECLARE_MULTICAST_DELEGATE_TwoParams(FBMOnLevelUpNative, int32 /*OldLevel*/, int32 /*NewLevel*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FBMOnXPChangedNative, float /*CurrentXP*/, float /*MaxXP*/, float /*Percent*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnSkillPointChangedNative, int32 /*NewSkillPoints*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FBMOnAttributePointChangedNative, int32 /*NewAttributePoints*/);

/**
 * 经验组件
 * 
 * 管理玩家的经验值、等级、技能点和属性点。
 * 处理升级、经验值获取和点数消耗等操作。
 */
UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMExperienceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBMExperienceComponent();

    /**
     * 添加经验值
     * 
     * @param Amount 添加的经验值数量
     * @return 升级的等级数量（如果经验值足够，可以返回大于1的值）
     */
    int32 AddXP(float Amount);

    /**
     * 检查玩家是否可以升级并处理升级
     * 
     * @return 升级的等级数量（0表示没有升级）
     */
    int32 CheckLevelUp();

    /**
     * 获取经验值百分比（0.0到1.0）用于UI显示
     * 
     * @return 当前经验值 / 下一级所需经验值
     */
    float GetExpPercent() const;

    
    /**
     * 消耗一个技能点
     * 
     * @return 是否消耗成功（true表示消耗成功，false表示没有技能点可用）
     */
    bool SpendSkillPoint();

    /**
     * 消耗一个属性点
     * 
     * @return 是否消耗成功（true表示消耗成功，false表示没有属性点可用）
     */
    bool SpendAttributePoint();


    /**
     * 获取等级
     * 
     * @return 当前等级
     */
    int32 GetLevel() const { return Level; }

    /**
     * 获取当前经验值
     * 
     * @return 当前经验值
     */
    float GetCurrentXP() const { return CurrentXP; }

    /**
     * 获取下一级所需经验值
     * 
     * @return 下一级所需经验值
     */
    float GetMaxXPForNextLevel() const { return CalculateXPForLevel(Level + 1); }

    /**
     * 获取技能点数量
     * 
     * @return 技能点数量
     */
    int32 GetSkillPoints() const { return SkillPoints; }

    /**
     * 获取属性点数量
     * 
     * @return 属性点数量
     */
    int32 GetAttributePoints() const { return AttributePoints; }

    
    /**
     * 设置等级
     * 
     * @param NewLevel 新等级
     * @param bApplyGrowth 是否应用属性增长
     * @param bApplyGrowth If true, applies stat growth for the new level
     */
    void SetLevel(int32 NewLevel, bool bApplyGrowth = false);

    /**
     * 设置当前经验值
     * 
     * @param NewXP 新经验值
     * @param bCheckLevelUp 是否检查升级
     * @param bCheckLevelUp If true, checks for level up after setting
     */
    void SetCurrentXP(float NewXP, bool bCheckLevelUp = false);

    /**
     * 设置技能点数量
     * 
     * @param NewSkillPoints 新技能点数量
     */
    void SetSkillPoints(int32 NewSkillPoints);

    /**
     * 设置属性点数量
     * 
     * @param NewAttributePoints 新属性点数量
     */
    void SetAttributePoints(int32 NewAttributePoints);

    /**
     * 计算达到特定等级所需的总经验值
     * 
     * @param TargetLevel 目标等级
     * @return 达到目标等级所需的总经验值
     */
    float CalculateXPForLevel(int32 TargetLevel) const;

    /**
     * 计算从一级到下一级所需的经验值
     * 
     * @param FromLevel 起始等级
     * @return 达到 FromLevel + 1 所需的经验值
     */
    float CalculateXPForNextLevel(int32 FromLevel) const;

protected:
    virtual void BeginPlay() override;

private:
    /**
     * 处理单次升级
     * 
     * @return 是否升级成功（true表示升级成功，false表示升级失败）
     */
    bool PerformLevelUp();

    /**
     * 在升级时应用属性增长
     * 可以被重写或扩展以应用属性奖励
     */
    void ApplyLevelUpBonuses();

    /** 当前玩家等级 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Experience", meta = (AllowPrivateAccess = "true"))
    int32 Level = 1;

    /** 当前经验值 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Experience", meta = (AllowPrivateAccess = "true"))
    float CurrentXP = 0.0f;

    /** 可用技能点数量 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Experience", meta = (AllowPrivateAccess = "true"))
    int32 SkillPoints = 0;

    /** 可用属性点数量 */
    UPROPERTY(VisibleAnywhere, Category = "BM|Experience", meta = (AllowPrivateAccess = "true"))
    int32 AttributePoints = 0;

    /** 基础经验值（用于二级等级） */
    UPROPERTY(EditAnywhere, Category = "BM|Experience|Config", meta = (ClampMin = "1.0"))
    float BaseXPRequired = 100.0f;

    /** 经验值增长倍率（指数增长） */
    UPROPERTY(EditAnywhere, Category = "BM|Experience|Config", meta = (ClampMin = "1.0"))
    float XPGrowthMultiplier = 1.15f;

    /** 每级升级获得的技能点数量 */
    UPROPERTY(EditAnywhere, Category = "BM|Experience|Config", meta = (ClampMin = "0"))
    int32 SkillPointsPerLevel = 1;

    /** 每级升级获得的属性点数量 */
    UPROPERTY(EditAnywhere, Category = "BM|Experience|Config", meta = (ClampMin = "0"))
    int32 AttributePointsPerLevel = 1;

    /** 最大等级限制（0表示无限制） */
    UPROPERTY(EditAnywhere, Category = "BM|Experience|Config", meta = (ClampMin = "0"))
    int32 MaxLevel = 0;

public:
    /** 玩家升级事件 */
    FBMOnLevelUpNative OnLevelUpNative;

    /** 经验值变化事件（用于UI更新） */
    FBMOnXPChangedNative OnXPChangedNative;

    /** 技能点变化事件 */
    FBMOnSkillPointChangedNative OnSkillPointChangedNative;

    /** 属性点变化事件 */
    FBMOnAttributePointChangedNative OnAttributePointChangedNative;
};
