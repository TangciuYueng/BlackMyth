// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/BMTypes.h"
#include "BMSaveGameSubsystem.generated.h"

class UBMSaveData;
class ABMPlayerCharacter;
class UBMEventBusSubsystem;

/**
 * 存档游戏子系统日志分类
 * 
 * 用于输出存档游戏子系统的调试信息
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBMSave, Log, All);

/**
 * 存档游戏子系统
 * 
 * 负责管理游戏存档的保存、加载、验证等功能
 * 提供同步和异步（待实现）的存档操作接口
 */
UCLASS()
class BLACKMYTH_API UBMSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    /**
     * 保存游戏到指定槽位
     * 
     * @param Slot 存档槽位编号（0 为自动存档槽位）
     * @return 保存成功返回 true，失败返回 false
     */
    bool SaveGame(int32 Slot);

    /**
     * 从指定槽位加载游戏
     * 
     * @param Slot 存档槽位编号
     * @return 加载成功返回 true，失败返回 false
     */
    bool LoadGame(int32 Slot);

    /**
     * 自动保存到默认槽位（槽位 0）
     */
    void AutoSave();

    /**
     * 检查指定槽位的存档是否存在
     * 
     * @param Slot 存档槽位编号
     * @return 存档存在返回 true，否则返回 false
     */
    bool DoesSaveExist(int32 Slot);

    /**
     * 获取存档元信息（用于UI显示）
     * 
     * @param Slot 存档槽位编号
     * @param OutMeta 输出的元信息结构
     * @return 成功获取返回 true，失败返回 false
     */
    bool GetSaveMeta(int32 Slot, FBMSaveMeta& OutMeta);

    /**
     * 删除指定槽位的存档
     * 
     * @param Slot 存档槽位编号
     * @return 删除成功返回 true，失败返回 false
     */
    bool DeleteSave(int32 Slot);

    /**
     * 获取所有存档槽位信息（用于UI显示存档列表）
     * 
     * @param MaxSlots 最大槽位数（默认10）
     * @return 存档槽位信息数组
     */
    TArray<FBMSaveSlotInfo> GetAllSaveSlots(int32 MaxSlots = 10);

protected:
    /**
     * 辅助函数：格式化槽位名称
     * 
     * @param Slot 槽位编号
     * @return 格式化的槽位名称（如 "SaveSlot_0"）
     */
    FString GetSlotName(int32 Slot) const;

    /**
     * 辅助函数：获取当前玩家角色
     * 
     * @return 当前玩家角色指针，如果不存在则返回 nullptr
     */
    ABMPlayerCharacter* GetPlayerCharacter() const;

    /**
     * 内部逻辑：将游戏世界数据写入 SaveData 对象
     * 
     * 收集玩家角色的所有需要保存的数据，包括：
     * - 位置和旋转
     * - 完整属性（HP、MP、Stamina、Attack、Defense 等）
     * - 经验和等级
     * - 背包物品
     * 
     * @param SaveData 要写入的存档数据对象
     * @param Player 玩家角色指针
     */
    void WriteSaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player);

    /**
     * 内部逻辑：将 SaveData 对象的数据应用回游戏世界
     * 
     * 恢复玩家角色的所有保存的数据，包括：
     * - 位置和旋转
     * - 完整属性
     * - 经验和等级
     * - 背包物品
     * 
     * @param SaveData 要应用的存档数据对象
     * @param Player 玩家角色指针
     */
    void ApplySaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player);

    /**
     * 验证存档数据有效性
     * 
     * @param SaveData 要验证的存档数据
     * @return 数据有效返回 true，否则返回 false
     */
    bool ValidateSaveData(const UBMSaveData* SaveData) const;

    /**
     * 获取事件总线子系统（用于发送通知）
     * 
     * @return 事件总线子系统指针
     */
    UBMEventBusSubsystem* GetEventBusSubsystem() const;

private:
    /** 默认自动存档槽位编号 */
    static const int32 AUTO_SAVE_SLOT = 0;
};
