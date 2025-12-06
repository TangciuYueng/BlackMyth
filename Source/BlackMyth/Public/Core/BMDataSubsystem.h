#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/BMSkillData.h"        
#include "Data/BMSceneData.h"
#include "Data/BMElementalData.h"
#include "Data/BMPlayerGrowthData.h"
#include "BMDataSubsystem.generated.h"

UCLASS()
class BLACKMYTH_API UBMDataSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 系统初始化
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // --- 查询接口 (API) ---

    // 获取技能数据
    const FBMSkillData* GetSkillData(FName SkillID) const;

    // 获取场景数据
    const FBMSceneData* GetSceneData(FName SceneID) const;

    // 获取玩家成长数据
    const FBMPlayerGrowthData* GetPlayerGrowthData(int32 Level) const;

    // 获取元素倍率
    float GetElementalMultiplier(FName AttackElement, FName DefendElement) const;

protected:
    // 缓存加载好的 DataTable 指针
    UPROPERTY()
    UDataTable* SkillTableCache;

    UPROPERTY()
    UDataTable* SceneTableCache;

    UPROPERTY()
    UDataTable* ElementTableCache;

    UPROPERTY()
    UDataTable* PlayerGrowthTableCache;

private:
    // 内部通用查找函数
    template <typename T>
    T* FindRow(UDataTable* Table, FName RowName) const;
};