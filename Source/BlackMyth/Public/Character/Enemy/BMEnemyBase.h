#pragma once

#include "CoreMinimal.h"
#include "Character/BMCharacterBase.h"
#include "Core/BMTypes.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "BMEnemyBase.generated.h"

class APawn;
class UAnimSequence;

UCLASS()
class BLACKMYTH_API ABMEnemyBase : public ABMCharacterBase
{
    GENERATED_BODY()

public:
    ABMEnemyBase();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    // ===== 类图接口 =====
    virtual bool DetectPlayer() const;
    virtual void DropLoot();
    virtual void SetAlertState(bool bAlert);

    // ===== FSM/AI 需要的“只读接口” =====
    bool IsAlerted() const { return bIsAlert; }
    APawn* GetCurrentTarget() const { return CurrentTarget.Get(); }
    bool HasValidTarget() const { return CurrentTarget.IsValid(); }

    float GetAggroRange() const { return AggroRange; }
    float GetPatrolRadius() const { return PatrolRadius; }
    float GetExpReward() const { return ExpReward; }
	float GetPatrolSpeed() const { return PatrolSpeed; }
	float GetChaseSpeed() const { return ChaseSpeed; }
    FVector GetHomeLocation() const { return HomeLocation; }
	float GetLocomotionSpeedThreshold() const { return LocomotionSpeedThreshold; }

    // 攻击判定
    bool IsInAttackRange() const;
    bool CanStartAttack() const;

    // 动画播放（与 Player 风格一致：Loop/Once）
    virtual void PlayIdleLoop();
    virtual void PlayWalkLoop();
    virtual void PlayRunLoop();
    virtual float PlayHitOnce(const FBMDamageInfo& Info);
    virtual float PlayDeathOnce();
    virtual float PlayAttackOnce(const FBMEnemyAttackSpec& Spec);
    virtual float PlayDodgeOnce();
    FVector ComputeBackwardDodgeDirFromHit(const FBMDamageInfo& InInfo) const;

    void SetSingleNodePlayRate(float Rate);
    // 出手选择
    bool SelectRandomAttackForCurrentTarget(FBMEnemyAttackSpec& OutSpec) const;

    // 攻击完成后由 AttackState 调用：推进冷却
    void CommitAttackCooldown(float CooldownSeconds);

    void SetActiveAttackSpec(const FBMEnemyAttackSpec& Spec);
    void ClearActiveAttackSpec();
    bool HasActiveAttackSpec() const { return bHasActiveAttackSpec; }
    const FBMEnemyAttackSpec& GetActiveAttackSpec() const { return ActiveAttackSpec; }
    const FBMDamageInfo& GetLastDamageInfo() const { return LastDamageInfo; }
    virtual bool ResolveHitBoxWindow(
        FName WindowId,
        TArray<FName>& OutHitBoxNames,
        FBMHitBoxActivationParams& OutParams
    ) const override;

    // 受击/死亡：由 HandleDamageTaken/HandleDeath 触发，最终切换到 Hit/Death 状态
    void RequestHitState(const FBMDamageInfo& FinalInfo);
    void RequestDeathState(const FBMDamageInfo& LastHitInfo);

    // 打断判定（工程化：集中一处）
    bool ShouldInterruptCurrentAttack(const FBMDamageInfo& Incoming) const;

    // 运动控制（由 State 调用，Controller 负责执行 MoveTo）
    bool RequestMoveToTarget(float AcceptanceRadius);
    bool RequestMoveToLocation(const FVector& Location, float AcceptanceRadius);
    void RequestStopMovement();

    // 朝向（攻击前/追击时可用）
    void FaceTarget(float DeltaSeconds, float TurnSpeedDeg = 720.f);

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeDistance = 420.f;               

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeOnHitChance = 0.3f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeCooldown = 2.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeSpeed = 850.f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgePlayRate = 1.0f;

    UPROPERTY(Transient)
    FVector DodgeLockedDir = FVector::BackwardVector;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    FName DodgeCooldownKey = TEXT("Enemy_Dodge");
protected:
    // 伤害链路：仍走 ABMCharacterBase（HitBox->TakeDamageFromHit->Stats）
    virtual void HandleDamageTaken(const FBMDamageInfo& FinalInfo) override;
    virtual void HandleDeath(const FBMDamageInfo& LastHitInfo) override;
    virtual bool CanBeDamagedBy(const FBMDamageInfo& Info) const override;
    virtual bool TryEvadeIncomingHit(const FBMDamageInfo& InInfo) override;

    // 内部动画工具
    void PlayLoop(UAnimSequence* Seq, float PlayRate = 1.0f);
    float PlayOnce(
        UAnimSequence* Seq,
        float PlayRate = 1.0f,
        float StartTime = 0.0f,
        float MaxPlayTime = -1.0f
    );
protected:
    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float AggroRange = 800.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float PatrolRadius = 400.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float ExpReward = 15.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    TArray<FBMLootItem> LootTable;

    // ===== 工程性：FSM 驱动的资产配置（建议由派生类/编辑器配置）=====
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimIdle = nullptr;

    // 动画资产：巡逻 Walk / 追击 Run / 受击 Light&Heavy / 死亡 Death
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimWalk = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimRun = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimHitLight = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimHitHeavy = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimDeath = nullptr;

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

    // 感知刷新频率（只更新 Target/Alert，不做状态切换）
    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Perception")
    float PerceptionInterval = 0.2f;

private:
    void InitEnemyStates();
    void CachePlayerPawn();
    void StartPerceptionTimer();
    void UpdatePerception();



private:
    TWeakObjectPtr<APawn> CachedPlayer;
    TWeakObjectPtr<APawn> CurrentTarget;

    bool bIsAlert = false;

    FVector HomeLocation = FVector::ZeroVector;

    // Loop 动画去重
    UPROPERTY(Transient)
    TObjectPtr<UAnimSequence> CurrentLoopAnim = nullptr;
    float CurrentLoopRate = 1.0f;
    // 攻击冷却
    float NextAttackAllowedTime = 0.f;

    FTimerHandle PerceptionTimerHandle;
};
