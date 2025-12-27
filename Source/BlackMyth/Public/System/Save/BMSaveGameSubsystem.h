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
     * 获取下一个可用的存档槽位
     * 
     * @return 下一个可用的存档槽位编号
     */
    int32 GetNextAvailableSlot();

    /**
     * 开始新游戏
     */
    void StartNewGame();

    /**
     * 保存游戏到指定槽位
     * 
     * @param Slot 存档槽位编号（0 为自动存档，1 为手动存档）
     * @return 保存成功返回 true，失败返回 false
     */
    bool SaveGame(int32 Slot);

    /**
     * 保存游戏到手动存档槽位（槽位1）
     * 
     * @return 保存成功返回 true，失败返回 false
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Save")
    bool SaveGameManual();

    /**
     * 从手动存档槽位（槽位1）加载游戏
     * 
     * @return 加载成功返回 true，失败返回 false
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Save")
    bool LoadGameManual();

    /**
     * 检查是否存在手动存档
     * 
     * @return 存在返回 true，否则返回 false
     */
    UFUNCTION(BlueprintCallable, Category = "BM|Save")
    bool HasManualSave() const;

    /**
     * 保存游戏到当前槽位
     * 
     * @return 保存成功返回 true，失败返回 false
     */
    bool SaveCurrentGame();

    /**
     * 从指定槽位加载游戏
     * 
     * @param Slot 存档槽位编号
     * @param bRestorePosition 是否恢复玩家位置（默认 true）
     * @return 加载成功返回 true，失败返回 false
     */
    bool LoadGame(int32 Slot, bool bRestorePosition = true);

    /**
     * 从指定槽位加载游戏（从主菜单调用，会先切换地图）
     * 
     * @param Slot 存档槽位编号
     * @return 加载成功返回 true，失败返回 false
     */
    bool LoadGameFromMainMenu(int32 Slot);

    /**
     * 获取存档中保存的地图名称
     * 
     * @param Slot 存档槽位编号
     * @param OutMapName 输出的地图名称
     * @return 成功获取返回 true，失败返回 false
     */
    bool GetSaveMapName(int32 Slot, FName& OutMapName);

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
    bool DoesSaveExist(int32 Slot) const;

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
    /** 当前存档槽位索引 */
    int32 CurrentSlotIndex = 1;

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
     * - 位置和旋转（可选）
     * - 完整属性
     * - 经验和等级
     * - 背包物品
     * 
     * @param SaveData 要应用的存档数据对象
     * @param Player 玩家角色指针
     * @param bRestorePosition 是否恢复位置（默认 true）
     */
    void ApplySaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player, bool bRestorePosition = true);

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
    
    /** 手动存档槽位编号 */
    static const int32 MANUAL_SAVE_SLOT = 1;
    
    /** 最大存档槽位数（仅用于兼容旧代码） */
    static const int32 MAX_SAVE_SLOTS = 50;

    /** 待恢复的存档槽位（地图切换后使用） */
    int32 PendingLoadSlot = -1;

    /** 待恢复的目标地图名称（用于验证地图切换是否正确） */
    FName PendingLoadMapName = NAME_None;

    /** 地图切换完成后的回调句柄 */
    FDelegateHandle PostLoadMapDelegateHandle;

    /** 处理地图切换完成后的存档恢复 */
    void HandlePostLoadMapForSave(UWorld* LoadedWorld);
};
