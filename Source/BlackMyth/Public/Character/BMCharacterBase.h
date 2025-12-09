#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/BMTypes.h"
#include "BMCharacterBase.generated.h"

class UBMStatsComponent;
class UBMCombatComponent;
class UBMStateMachineComponent;
class UBMAnimEventComponent;
class UBMHitBoxComponent;
class UBMHurtBoxComponent;
class UAnimMontage;
class UPrimitiveComponent;

/**
 * 角色系统日志分类
 *
 * 用于输出角色生命周期、受击结算、状态切换等调试信息
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBMCharacter, Log, All);


/**
 * 角色受击事件
 *
 * 在 TakeDamageFromHit 完成结算并得到最终伤害信息后触发，外部系统
 * 可订阅该事件获取最终的 FBMDamageInfo
 *
 * 参数说明：
 * - Victim：受击者角色
 * - FinalInfo：最终结算后的伤害信息
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FBMOnCharacterDamaged, class ABMCharacterBase* /*Victim*/, const FBMDamageInfo& /*FinalInfo*/);

/**
 * 角色死亡事件
 *
 * 参数说明：
 * - Victim：死亡角色
 * - LastHitInfo：导致死亡的最后一次有效伤害信息
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FBMOnCharacterDied, class ABMCharacterBase* /*Victim*/, const FBMDamageInfo& /*LastHitInfo*/);

UCLASS(Abstract)
/**
 * 角色基类
 *
 * 提供所有角色共用的核心能力：
 * - 组件聚合（Stats/Combat/FSM/AnimEvent/HitBox/HurtBox）
 * - 统一受击入口 TakeDamageFromHit
 * - 受击/死亡事件广播
 */
class BLACKMYTH_API ABMCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 初始化组件引用、默认身份属性，以及与受击/状态机相关的基础设置
     */
    ABMCharacterBase();

    /**
     * 每帧更新回调
     *
     * 通常用于驱动状态机 Tick、持续性效果或调试输出等
     *
     * @param DeltaSeconds 帧时间间隔
     */
    virtual void Tick(float DeltaSeconds) override;

    /**
     * 生命周期回调：角色进入关卡后调用
     *
     * 用于完成运行时初始化
     */
    virtual void BeginPlay() override;

    /**
     * 统一受击结算入口（使用 FBMDamageInfo）
     *
     * 典型流程：
     * 1) CanBeDamagedBy：检查阵营/无敌/死亡等条件
     * 2) 计算倍率（命中 HurtBox、元素克制/抗性、暴击等）
     * 3) 扣减 Stats（HP 等）
     * 4) HandleDamageTaken：派生类可插入受击表现（动画/特效/硬直）
     * 5) 广播 OnCharacterDamaged
     * 6) 若死亡则进入 HandleDeath 并广播 OnCharacterDied
     *
     * 注意：InOutInfo 为引用参数，函数可能会回填最终伤害值、受击反馈类型、命中点等信息，
     * 便于 UI/事件系统复用同一份数据
     *
     * @param InOutInfo 伤害信息（输入为原始命中信息，输出为最终结算信息）
     * @return 实际生效的伤害值（最终扣减的 HP）
     */
    virtual float TakeDamageFromHit(FBMDamageInfo& InOutInfo);

    /**
     * 获取角色当前前向向量
     *
     * 默认可直接返回 ActorForwardVector
     *
     * @return 世界空间前向单位向量
     */
    virtual FVector GetForwardVector() const;

    /**
     * 获取数值组件（Stats）
     *
     * @return Stats 组件指针
     */
    UBMStatsComponent* GetStats() const { return Stats; }

    /**
     * 获取战斗组件（Combat）
     *
     * @return Combat 组件指针
     */
    UBMCombatComponent* GetCombat() const { return Combat; }

    /**
     * 获取状态机组件（FSM）
     *
     * @return FSM 组件指针
     */
    UBMStateMachineComponent* GetFSM() const { return FSM; }

    /**
     * 获取动画事件组件（AnimEvent）
     *
     * @return AnimEvent 组件指针
     */
    UBMAnimEventComponent* GetAnimEvent() const { return AnimEvent; }

    /**
     * 获取攻击判定组件（HitBox）
     *
     * @return HitBox 组件指针
     */
    UBMHitBoxComponent* GetHitBox() const { return HitBox; }

    /**
     * 角色受击事件
     *
     * 在一次伤害结算完成后触发，携带最终 FBMDamageInfo
     */
    FBMOnCharacterDamaged OnCharacterDamaged;

    /**
     * 角色死亡事件
     *
     * 在角色死亡流程开始时触发，携带导致死亡的最后一次有效伤害信息
     */
    FBMOnCharacterDied OnCharacterDied;

    /**
     * 角色阵营
     *
     * 用于友伤判定、AI 目标选择以及事件过滤等。默认中立
     */
    UPROPERTY(EditAnywhere, Category = "BM|Identity")
    EBMTeam Team = EBMTeam::Neutral;

    /**
     * 角色类型（玩家/敌人/Boss）
     *
     * 用于通用逻辑分支
     */
    UPROPERTY(EditAnywhere, Category = "BM|Identity")
    EBMCharacterType CharacterType = EBMCharacterType::Enemy;

protected:

    /**
     * 判断该角色是否可被当前伤害信息影响
     *
     * @param Info 伤害信息
     * @return 若允许受到该伤害则返回 true，否则返回 false
     */
    virtual bool CanBeDamagedBy(const FBMDamageInfo& Info) const;

    /**
     * 处理受击后的表现与副作用
     *
     * @param FinalInfo 最终结算后的伤害信息
     */
    virtual void HandleDamageTaken(const FBMDamageInfo& FinalInfo);

    /**
     * 处理死亡流程
     *
     * @param LastHitInfo 导致死亡的最后一次有效伤害信息
     */
    virtual void HandleDeath(const FBMDamageInfo& LastHitInfo);

    /**
     * 根据命中的组件计算伤害倍率
     *
     * 默认用于 HurtBox 体系
     *
     * @param HitComponent 命中的碰撞组件/骨骼附属组件
     * @return 伤害倍率，默认为 1.0
     */
    virtual float GetDamageMultiplierForComponent(const UPrimitiveComponent* HitComponent) const;

protected:
    /**
     * 数值组件（Stats）
     *
     * 管理 HP/体力/攻击/防御等属性，并提供扣血与死亡判定等能力
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMStatsComponent> Stats;

    /**
     * 战斗组件（Combat）
     *
     * 管理攻击请求、技能触发、命中列表与动作锁等战斗相关逻辑
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMCombatComponent> Combat;

    /**
     * 状态机组件（FSM）
     *
     * 驱动角色行为状态（Idle/Move/Jump/Attack/Hit/Death 等）的切换与更新
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMStateMachineComponent> FSM;

    /**
     * 动画事件组件（AnimEvent）
     *
     * 用于从动画关键帧触发逻辑（如开启/关闭 HitBox、播放特效、发布事件等）
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMAnimEventComponent> AnimEvent;

    /**
     * 攻击判定组件（HitBox）
     *
     * 管理角色攻击时的命中检测与伤害信息构建，并与 FBMDamageInfo 体系对接
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMHitBoxComponent> HitBox;

    /**
     * 受击判定组件列表（HurtBoxes）
     *
     * 用于根据被命中的部位/组件计算伤害倍率、弱点/抗性等
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TArray<TObjectPtr<UBMHurtBoxComponent>> HurtBoxes;

private:
    /**
     * 缓存角色身上的 HurtBox 组件集合
     *
     * 用于在运行时快速查询命中部位对应的倍率与属性，避免每次受击动态查找
     */
    void CacheHurtBoxes();

    /**
     * 处理 Stats 组件触发的死亡回调
     *
     * 当 Stats 判定生命值归零时调用，用于统一进入死亡流程并触发死亡事件广播
     *
     * @param Killer 造成死亡的施害者
     */
    void HandleStatsDeath(AActor* Killer);

private:
    /**
     * 最近一次实际造成扣血的伤害信息缓存
     *
     * 用于死亡流程与事件广播
     */
    FBMDamageInfo LastAppliedDamageInfo;
};
