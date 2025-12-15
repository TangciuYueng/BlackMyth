#pragma once

#include "CoreMinimal.h"
#include "Character/BMCharacterBase.h"
#include "BMEnemyBase.generated.h"

class UAnimSequence;
class APawn;
class UBMHealthBarComponent;

/**
 * 基础敌人角色，实现最小可用的敌人逻辑：
 * - 纯 C++ 配置网格与待机动画
 * - 保持待机动画循环，供玩家攻击测试
 * - 提供 Aggro 检测、战利品掉落与警戒状态控制
 */
UCLASS()
class BLACKMYTH_API ABMEnemyBase : public ABMCharacterBase
{
    GENERATED_BODY()

public:
    ABMEnemyBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /**
     * 简易感知：检测玩家是否进入仇恨范围
     */
    virtual bool DetectPlayer() const;

    /**
     * 掉落逻辑（基础实现仅打印日志）
     */
    virtual void DropLoot();

    /**
     * 设置敌人是否处于警戒状态
     */
    virtual void SetAlertState(bool bAlert);

    bool IsAlerted() const { return bIsAlert; }

protected:
    virtual void HandleDamageTaken(const FBMDamageInfo& FinalInfo) override;
    virtual void HandleDeath(const FBMDamageInfo& LastHitInfo) override;
    virtual bool CanBeDamagedBy(const FBMDamageInfo& Info) const override;

    /**
     * 循环播放待机动画
     */
    void PlayIdleLoop();

    /**
     * 每帧简单感知刷新
     */
    virtual void UpdatePerception(float DeltaSeconds);

    /**
     * 缓存玩家 Pawn 引用，避免频繁查找
     */
    void CachePlayerPawn();

protected:
    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float AggroRange = 800.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float PatrolRadius = 400.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    float ExpReward = 15.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    EBMEnemyType EnemyType = EBMEnemyType::Goblin;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Data")
    FName EnemyDataId = TEXT("Enemy_DemonWolf");

    UPROPERTY(EditAnywhere, Category = "BM|Enemy")
    TArray<FBMLootItem> LootTable;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Combat")
    float BaseDamage = 12.f;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<USkeletalMesh> EnemyMeshAsset = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Assets")
    TObjectPtr<UAnimSequence> AnimIdle = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Enemy|Debug")
    bool bDrawAggroRange = false;

    UPROPERTY(VisibleAnywhere, Category = "BM|Enemy|Components")
    TObjectPtr<UBMHealthBarComponent> HealthBarComponent = nullptr;

private:
    void InitializeHurtBoxes();
    void InitializeHitBoxes();
    void LoadEnemyDataAssets();
    void ApplyConfiguredAssets();
    FName ResolveEnemyDataId() const;

private:
    TWeakObjectPtr<APawn> CachedPlayer;
    bool bIsAlert = false;
};
