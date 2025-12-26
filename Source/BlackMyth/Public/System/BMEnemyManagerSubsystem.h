#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/BMTypes.h"
#include "BMEnemyManagerSubsystem.generated.h"

class ABMEnemyBase;

/**
 * 敌人存活状态变化事件
 * 
 * @param AliveCount 当前存活敌人数量
 * @param TotalCount 总注册敌人数量
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FBMOnEnemyCountChanged, int32 /*AliveCount*/, int32 /*TotalCount*/);

/**
 * 敌人管理子系统
 * 
 * 负责追踪场景中所有敌人的生存状态，提供统一查询接口
 */
UCLASS()
class BLACKMYTH_API UBMEnemyManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * 注册敌人到管理系统
     * 
     * @param Enemy 需要注册的敌人实例
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    void RegisterEnemy(ABMEnemyBase* Enemy);

    /**
     * 注销敌人
     * 
     * @param Enemy 需要注销的敌人实例
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    void UnregisterEnemy(ABMEnemyBase* Enemy);

    /**
     * 获取当前存活敌人数量
     * 
     * @return 存活敌人数量
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    int32 GetAliveEnemyCount() const;

    /**
     * 获取总注册敌人数量
     * 
     * @return 总敌人数量
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    int32 GetTotalEnemyCount() const;

    /**
     * 获取所有存活敌人列表
     * 
     * @param OutEnemies 输出存活敌人数组
     * @return 存活敌人数量
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    int32 GetAliveEnemies(TArray<ABMEnemyBase*>& OutEnemies) const;

    /**
     * 获取所有敌人列表（包括死亡但未销毁的）
     * 
     * @param OutEnemies 输出敌人数组
     * @return 敌人数量
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    int32 GetAllEnemies(TArray<ABMEnemyBase*>& OutEnemies) const;

    /**
     * 判断是否所有敌人都已死亡
     * 
     * @return 若所有敌人都死亡则返回 true
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    bool AreAllEnemiesDead() const;

    /**
     * 清理无效引用
     * 
     * 系统会自动定期清理
     */
    UFUNCTION(BlueprintCallable, Category = "BM|EnemyManager")
    void CleanupInvalidEnemies();

    /**
     * 敌人数量变化事件
     * 
     * 当敌人注册、死亡或注销时触发
     */
    FBMOnEnemyCountChanged OnEnemyCountChanged;

private:
    /**
     * 处理敌人死亡事件
     * 
     * @param Victim 死亡的敌人
     * @param LastHitInfo 最后一次伤害信息
     */
    void HandleEnemyDeath(class ABMCharacterBase* Victim, const FBMDamageInfo& LastHitInfo);

    /**
     * 广播敌人数量变化事件
     */
    void BroadcastCountChanged();

    /**
     * 检查关卡完成条件并执行关卡切换
     */
    void CheckLevelCompletionAndTransition();

    /**
     * 延迟切换到下一关卡
     */
    void TransitionToNextLevel();

private:
    /** 注册的所有敌人 */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<ABMEnemyBase>> RegisteredEnemies;

    /** 自动清理计时器 */
    FTimerHandle CleanupTimerHandle;

    /** 关卡切换延迟计时器 */
    FTimerHandle LevelTransitionTimerHandle;

    /** 清理间隔（秒） */
    UPROPERTY(EditAnywhere, Category = "BM|EnemyManager")
    float CleanupInterval = 5.0f;

    /** 是否已触发关卡切换 */
    bool bLevelTransitionTriggered = false;
};
