#pragma once

#include "CoreMinimal.h"
#include "Character/BMCharacterBase.h"
#include "Core/BMTypes.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "BMEnemyBase.generated.h"

class APawn;
class UAnimSequence;
class UBMEnemyHealthBarComponent;

/**
 * 敌人基类
 *
 * 所有敌人的基础类，提供：
 * - 完整的 AI 状态机
 * - 玩家检测和追击逻辑
 * - 攻击选择和冷却管理
 * - 动画播放（循环/单次）
 * - 受击打断判定和闪避机制
 * - 掉落物系统
 * - 悬浮血条管理
 * - DataTable 数据驱动配置
 */
UCLASS()
class BLACKMYTH_API ABMEnemyBase : public ABMCharacterBase
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 初始化 AI 控制器、动画模式和移动参数
     */
    ABMEnemyBase();

    /**
     * 开始游戏生命周期
     *
     * 初始化家位置、玩家缓存、感知定时器、状态机和悬浮血条
     */
    virtual void BeginPlay() override;

    /**
     * 每帧更新
     *
     * @param DeltaSeconds 帧时间间隔
     */
    virtual void Tick(float DeltaSeconds) override;

    // ===== 类图接口 =====
    
    /**
     * 检测玩家是否在警戒范围内
     *
     * @return 玩家在范围内且存活返回 true
     */
    virtual bool DetectPlayer() const;

    /**
     * 掉落战利品
     *
     * 向玩家发放金币、经验和物品，并通过事件总线显示通知
     */
    virtual void DropLoot();

    /**
     * 设置警戒状态
     *
     * @param bAlert true 进入警戒，false 解除警戒
     */
    virtual void SetAlertState(bool bAlert);

    // ===== FSM/AI 只读接口 =====
    
    /** 查询是否处于警戒状态 */
    bool IsAlerted() const { return bIsAlert; }
    
    /** 获取当前追击目标 */
    APawn* GetCurrentTarget() const { return CurrentTarget.Get(); }
    
    /** 查询是否有有效目标 */
    bool HasValidTarget() const { return CurrentTarget.IsValid(); }

    /** 获取警戒范围 */
    float GetAggroRange() const { return AggroRange; }
    
    /** 获取巡逻半径 */
    float GetPatrolRadius() const { return PatrolRadius; }
    
    /** 获取巡逻速度 */
	float GetPatrolSpeed() const { return PatrolSpeed; }
	
	/** 获取追击速度 */
	float GetChaseSpeed() const { return ChaseSpeed; }
	
	/** 获取家位置（出生位置）*/
    FVector GetHomeLocation() const { return HomeLocation; }
    
    /** 获取移动速度阈值（低于此速度强制 Idle）*/
	float GetLocomotionSpeedThreshold() const { return LocomotionSpeedThreshold; }

    // ===== 攻击判定 =====
    
    /**
     * 判断目标是否在任意攻击范围内
     *
     * @return 在某个攻击范围内返回 true
     */
    bool IsInAttackRange() const;
    
    /**
     * 判断是否可以开始攻击
     *
     * 检查全局冷却、目标有效性、不在空中、目标在攻击范围且冷却就绪
     *
     * @return 满足所有条件返回 true
     */
    bool CanStartAttack() const;

    // ===== 动画播放 =====
    
    /** 播放待机循环动画 */
    virtual void PlayIdleLoop();
    
    /** 播放行走循环动画 */
    virtual void PlayWalkLoop();
    
    /** 播放奔跑循环动画 */
    virtual void PlayRunLoop();
    
    /**
     * 播放受击动画一次
     *
     * 根据伤害类型选择轻击或重击动画
     *
     * @param Info 伤害信息
     * @return 动画时长（秒）
     */
    virtual float PlayHitOnce(const FBMDamageInfo& Info);
    
    /**
     * 播放死亡动画一次
     *
     * @return 动画时长（秒）
     */
    virtual float PlayDeathOnce();
    
    /**
     * 播放指定攻击动画一次
     *
     * @param Spec 攻击规格
     * @return 动画时长（秒）
     */
    virtual float PlayAttackOnce(const FBMEnemyAttackSpec& Spec);
    
    /**
     * 播放闪避动画一次
     *
     * @return 动画时长（秒）
     */
    virtual float PlayDodgeOnce();
    
    /**
     * 根据伤害来源计算后撤闪避方向
     *
     * @param InInfo 伤害信息
     * @return 归一化的闪避方向向量
     */
    FVector ComputeBackwardDodgeDirFromHit(const FBMDamageInfo& InInfo) const;

    /**
     * 设置单节点动画播放速率
     *
     * @param Rate 播放速率（1.0 = 正常速度）
     */
    void SetSingleNodePlayRate(float Rate);
    // ===== 攻击选择与冷却 =====
    
    /**
     * 为当前目标随机选择攻击
     *
     * 基于距离、权重和冷却筛选可用攻击
     *
     * @param OutSpec 输出选中的攻击规格
     * @return 找到可用攻击返回 true
     */
    bool SelectRandomAttackForCurrentTarget(FBMEnemyAttackSpec& OutSpec) const;

    /**
     * 提交攻击全局冷却
     *
     * 设置下次允许攻击的时间，带随机浮动
     *
     * @param CooldownSeconds 冷却时间
     */
    void CommitAttackCooldown(float CooldownSeconds);

    /**
     * 设置当前激活的攻击规格
     *
     * @param Spec 攻击规格
     */
    void SetActiveAttackSpec(const FBMEnemyAttackSpec& Spec);
    
    /** 清除当前激活的攻击规格 */
    void ClearActiveAttackSpec();
    
    /** 查询是否有激活的攻击规格 */
    bool HasActiveAttackSpec() const { return bHasActiveAttackSpec; }
    
    /** 获取当前激活的攻击规格 */
    const FBMEnemyAttackSpec& GetActiveAttackSpec() const { return ActiveAttackSpec; }
    
    /** 获取最后一次伤害信息 */
    const FBMDamageInfo& GetLastDamageInfo() const { return LastDamageInfo; }
    
    /**
     * 解析 HitBox 窗口配置
     *
     * 从当前激活的攻击规格获取 HitBox 配置
     *
     * @param WindowId 窗口 ID
     * @param OutHitBoxNames 输出 HitBox 名称列表
     * @param OutParams 输出 HitBox 激活参数
     * @return 成功解析返回 true
     */
    virtual bool ResolveHitBoxWindow(
        FName WindowId,
        TArray<FName>& OutHitBoxNames,
        FBMHitBoxActivationParams& OutParams
    ) const override;

    // ===== 状态请求与打断判定 =====
    
    /**
     * 请求切换到受击状态
     *
     * 根据当前状态和打断概率决定是否切换
     *
     * @param FinalInfo 最终伤害信息
     */
    void RequestHitState(const FBMDamageInfo& FinalInfo);
    
    /**
     * 请求切换到死亡状态
     *
     * @param LastHitInfo 致死伤害信息
     */
    void RequestDeathState(const FBMDamageInfo& LastHitInfo);

    /**
     * 判断当前攻击是否应被打断
     *
     * 基于霸体标记和打断概率判定
     *
     * @param Incoming 传入的伤害信息
     * @return 应该打断返回 true
     */
    bool ShouldInterruptCurrentAttack(const FBMDamageInfo& Incoming) const;

    // ===== 运动控制 =====
    
    /**
     * 请求移动到当前目标
     *
     * @param AcceptanceRadius 接受半径
     * @return 请求成功返回 true
     */
    bool RequestMoveToTarget(float AcceptanceRadius);
    
    /**
     * 请求移动到指定位置
     *
     * @param Location 目标位置
     * @param AcceptanceRadius 接受半径
     * @return 请求成功返回 true
     */
    bool RequestMoveToLocation(const FVector& Location, float AcceptanceRadius);
    
    /** 请求停止移动 */
    void RequestStopMovement();

    /**
     * 平滑朝向目标
     *
     * @param DeltaSeconds 帧时间间隔
     * @param TurnSpeedDeg 转向速度（度/秒）
     */
    void FaceTarget(float DeltaSeconds, float TurnSpeedDeg = 720.f);

    // ===== DataTable 数据加载 =====
    
    /**
     * 从 DataTable 加载属性和配置
     *
     * 根据 GetEnemyDataID() 返回的 ID 加载数据
     * 在子类中重写 GetEnemyDataID() 指定敌人类型
     */
    virtual void LoadStatsFromDataTable();
    
    /**
     * 获取敌人数据 ID
     *
     * 在子类中重写以返回敌人标识符（如 "EnemyDummy"）
     *
     * @return 敌人数据 ID
     */
    virtual FName GetEnemyDataID() const { return NAME_None; }


protected:
    /**
     * 从 DataTable 数据加载资产
     *
     * 加载骨骼网格和动画，由 LoadStatsFromDataTable() 调用
     *
     * @param Data 敌人数据指针
     */
    void LoadAssetsFromDataTable(const struct FBMEnemyData* Data);

public:
    // ===== 闪避参数 =====
    
    /** 闪避移动距离 */
    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeDistance = 420.f;

    /** 受击时触发闪避的概率（0-1）*/
    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeOnHitChance = 0.3f;

    /** 闪避冷却时间（秒）*/
    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeCooldown = 2.0f;

    /** 闪避移动速度 */
    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeSpeed = 850.f;

    /** 闪避动画播放速率 */
    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgePlayRate = 1.0f;

    /** 当前闪避锁定方向 */
    UPROPERTY(Transient)
    FVector DodgeLockedDir = FVector::BackwardVector;

    /** 闪避冷却键名 */
    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    FName DodgeCooldownKey = TEXT("Enemy_Dodge");

    // ===== 悬浮血条 =====
    
    /**
     * 判断是否应显示悬浮血条
     *
     * 在子类中重写以禁用
     *
     * @return 默认返回 true
     */
    virtual bool ShouldShowFloatingHealthBar() const { return true; }

    /**
     * 获取悬浮血条组件
     *
     * @return 血条组件指针
     */
    UBMEnemyHealthBarComponent* GetFloatingHealthBar() const { return FloatingHealthBar; }

protected:
    // ===== 伤害处理链路 =====
    
    /**
     * 处理受到伤害
     *
     * 请求切换到 Hit 状态
     *
     * @param FinalInfo 最终伤害信息
     */
    virtual void HandleDamageTaken(const FBMDamageInfo& FinalInfo) override;
    
    /**
     * 处理死亡
     *
     * 请求切换到 Death 状态并掉落战利品
     *
     * @param LastHitInfo 致死伤害信息
     */
    virtual void HandleDeath(const FBMDamageInfo& LastHitInfo) override;
    
    /**
     * 判断是否可被该伤害击中
     *
     * 过滤友军伤害
     *
     * @param Info 伤害信息
     * @return 可被击中返回 true
     */
    virtual bool CanBeDamagedBy(const FBMDamageInfo& Info) const override;
    
    /**
     * 尝试闪避传入的攻击
     *
     * 基于概率和冷却判定是否触发闪避
     *
     * @param InInfo 传入的伤害信息
     * @return 成功闪避返回 true（不受伤）
     */
    virtual bool TryEvadeIncomingHit(const FBMDamageInfo& InInfo) override;

    // ===== 内部动画工具 =====
    
    /**
     * 播放循环动画
     *
     * 带去重逻辑，避免重复播放相同动画
     *
     * @param Seq 动画序列
     * @param PlayRate 播放速率
     */
    void PlayLoop(UAnimSequence* Seq, float PlayRate = 1.0f);
    
    /**
     * 播放单次动画
     *
     * 支持指定起始时间和最大播放时长
     *
     * @param Seq 动画序列
     * @param PlayRate 播放速率
     * @param StartTime 起始时间（秒）
     * @param MaxPlayTime 最大播放时长（<=0 表示播完）
     * @return 实际播放时长（秒）
     */
    float PlayOnce(
        UAnimSequence* Seq,
        float PlayRate = 1.0f,
        float StartTime = 0.0f,
        float MaxPlayTime = -1.0f
    );
protected:
    // ===== AI 参数 =====
    
    /** 警戒范围 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float AggroRange = 800.f;

    /** 巡逻半径（从家位置开始）*/
    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float PatrolRadius = 400.f;

    // ===== 掉落配置 =====
    
    /** 掉落物品表 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    TArray<FBMLootItem> LootTable;

    /** 最小掉落金币数 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0"))
    int32 CurrencyDropMin = 200;

    /** 最大掉落金币数 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0"))
    int32 CurrencyDropMax = 300;

    /** 最小掉落经验值 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0.0"))
    float ExpDropMin = 100.0f;

    /** 最大掉落经验值 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Loot", meta = (ClampMin = "0.0"))
    float ExpDropMax = 300.0f;

    // ===== 动画资产 =====
    
    /** 待机动画 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimIdle = nullptr;

    /** 行走动画（巡逻）*/
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimWalk = nullptr;

    /** 奔跑动画（追击）*/
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimRun = nullptr;

    /** 轻度受击动画 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimHitLight = nullptr;

    /** 重度受击动画 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimHitHeavy = nullptr;

    /** 死亡动画 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimDeath = nullptr;

    /** 闪避动画 */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimDodge = nullptr;
    // 受击信息：给 HitState 选择动画用
    UPROPERTY(Transient)
    FBMDamageInfo LastDamageInfo;

    // 当前攻击：给“能否被打断”判定用
    UPROPERTY(Transient)
    bool bHasActiveAttackSpec = false;

    UPROPERTY(Transient)
    FBMEnemyAttackSpec ActiveAttackSpec;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    TArray<FBMEnemyAttackSpec> AttackSpecs;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack", meta = (ClampMin = "0"))
    float AttackRangeOverride = -1.f; 

    // 攻击之间的全局最小间隔
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    float GlobalAttackInterval = 2.0f;

    // 全局间隔的随机浮动范围
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Attack")
    float GlobalAttackIntervalDeviation = 0.5f;

    // 移动速度（便于不同敌人复用）
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Move")
    float PatrolSpeed = 220.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Move")
    float ChaseSpeed = 420.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Anim", meta = (ClampMin = "0.0"))
    float LocomotionSpeedThreshold = 5.0f; // 速度小于该阈值时强制Idle

    // 感知刷新频率
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Perception")
    float PerceptionInterval = 0.2f;

    // ===== Floating Health Bar =====
    /** Floating health bar component displayed above enemy head. */
    UPROPERTY(VisibleAnywhere, Category = "BM|Enemy|UI")
    TObjectPtr<UBMEnemyHealthBarComponent> FloatingHealthBar = nullptr;

    /** Vertical offset for the floating health bar above capsule top. */
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|UI", meta = (ClampMin = "0.0"))
    float FloatingHealthBarOffset = 50.f;

private:
    // ===== 内部初始化函数 =====
    
    /** 初始化敌人状态机 */
    void InitEnemyStates();
    
    /** 缓存玩家 Pawn */
    void CachePlayerPawn();
    
    /** 启动感知定时器 */
    void StartPerceptionTimer();
    
    /** 更新感知（检测玩家并更新目标和警戒状态）*/
    void UpdatePerception();
    
    /** 初始化悬浮血条 */
    void InitFloatingHealthBar();



private:
    // ===== 私有运行时状态 =====
    
    /** 缓存的玩家 Pawn */
    TWeakObjectPtr<APawn> CachedPlayer;
    
    /** 当前追击目标 */
    TWeakObjectPtr<APawn> CurrentTarget;

    /** 是否处于警戒状态 */
    bool bIsAlert = false;

    /** 家位置（出生位置，用于巡逻中心点）*/
    FVector HomeLocation = FVector::ZeroVector;

    /** 当前播放的循环动画（用于去重）*/
    UPROPERTY(Transient)
    TObjectPtr<UAnimSequence> CurrentLoopAnim = nullptr;
    
    /** 当前循环动画播放速率 */
    float CurrentLoopRate = 1.0f;
    
    /** 下次允许攻击的时间（世界时间）*/
    float NextAttackAllowedTime = 0.f;

    /** 感知定时器句柄 */
    FTimerHandle PerceptionTimerHandle;
};

