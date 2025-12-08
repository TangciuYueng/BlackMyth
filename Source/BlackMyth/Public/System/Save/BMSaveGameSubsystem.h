// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BMSaveGameSubsystem.generated.h"

class UBMSaveData;
class ABMPlayerCharacter;

/**
 * Save Game subsystem to manage saving and loading game data.
 */
UCLASS()
class BLACKMYTH_API UBMSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
    /** 保存游戏到指定槽位 */
    bool SaveGame(int32 Slot);

    /** 从指定槽位加载游戏 */
    bool LoadGame(int32 Slot);

    /** 自动保存 (默认槽位) */
    void AutoSave();

    /** 检查存档是否存在 */
    bool DoesSaveExist(int32 Slot);

protected:
    /** 辅助函数：格式化槽位名称 */
    FString GetSlotName(int32 Slot) const;

    /** 辅助函数：获取当前玩家角色 */
    ABMPlayerCharacter* GetPlayerCharacter() const;

    /** 内部逻辑：将游戏世界数据写入 SaveData 对象 */
    void WriteSaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player);

    /** 内部逻辑：将 SaveData 对象的数据应用回游戏世界 */
    void ApplySaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player);
};
