#pragma once

#include "CoreMinimal.h"
#include "Character/ABMCharacterBase.h"
#include "BMTypes.h" // 包含掉落物结构体
#include "ABMEnemyBase.generated.h"

// 前置声明
class UBehaviorTree;
class ABMPlayerCharacter;

UCLASS()
class MYGAME_API AABMEnemyBase : public AABMCharacterBase
{
    GENERATED_BODY()

public:
    AABMEnemyBase();
	// 怪物 ID，用于去 DataSubsystem 查表
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (ExposeOnSpawn = "true"))
    FName EnemyID;

    // 初始化函数：应用数据表中的属性
    virtual void InitEnemyData();

    virtual void Tick(float DeltaTime) override;

    // --- AI 配置 ---
    
    // 行为树资产：在编辑器中为不同敌人指派不同的 AI 逻辑
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    // --- 感知参数 ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float AggroRange = 800.f; // 警戒范围

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float LoseAggroRange = 1200.f; // 脱战范围（通常比警戒范围大，防止边缘抖动）

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float PatrolRadius = 1000.f; // 巡逻半径

    // --- 游戏性奖励 ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float ExpReward = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    TArray<FBMLootItem> LootTable;

    // --- 核心方法 ---

    /** * 检测玩家
     * 返回查找到的玩家 Actor 指针，如果没有检测到则返回 nullptr 
     */
    UFUNCTION(BlueprintCallable, Category = "AI")
    AActor* DetectPlayer();

    /** 掉落物品逻辑 */
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void DropLoot();

protected:
    // 缓存玩家指针，避免每帧去搜索 World
    UPROPERTY()
    TObjectPtr<AActor> CachedPlayerTarget;

	virtual void BeginPlay() override;
};