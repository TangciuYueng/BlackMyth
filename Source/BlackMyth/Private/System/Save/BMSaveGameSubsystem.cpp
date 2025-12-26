#include "System/Save/BMSaveGameSubsystem.h"

#include "System/Save/BMSaveData.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMExperienceComponent.h"
#include "System/Event/BMEventBusSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

// 日志分类定义
DEFINE_LOG_CATEGORY(LogBMSave);

/**
 * 获取存档槽位名称
 * 
 * 根据槽位编号生成格式化的槽位名称
 * 
 * @param Slot 槽位编号
 * @return 格式化的槽位名称（如 "SaveSlot_0"）
 */
FString UBMSaveGameSubsystem::GetSlotName(int32 Slot) const
{
    // Naming rule: SaveSlot_0, SaveSlot_1, etc.
    return FString::Printf(TEXT("SaveSlot_%d"), Slot);
}

/**
 * 获取当前玩家角色
 * 
 * 获取 PlayerController 0 控制的 Pawn
 * 
 * @return 当前玩家角色指针，如果不存在则返回 nullptr
 */
ABMPlayerCharacter* UBMSaveGameSubsystem::GetPlayerCharacter() const
{
    // 获取 PlayerController 0 控制的 Pawn
    if (UWorld* World = GetWorld())
    {
        return Cast<ABMPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    }
    return nullptr;
}

/**
 * 检查指定槽位的存档是否存在
 * 
 * 检查指定槽位的存档是否存在
 * 
 * @param Slot 存档槽位编号
 * @return 存档存在返回 true，否则返回 false
 */
bool UBMSaveGameSubsystem::DoesSaveExist(int32 Slot) const
{
    if (Slot < 0)
    {
        UE_LOG(LogBMSave, Warning, TEXT("DoesSaveExist: Invalid slot number %d"), Slot);
        return false;
    }
    
    // 检查指定槽位的存档是否存在
    return UGameplayStatics::DoesSaveGameExist(GetSlotName(Slot), 0);
}

/**
 * 自动保存到默认槽位（槽位 0）
 * 
 * 自动保存到默认槽位（槽位 0）
 * 
 * @return 自动保存成功返回 true，失败返回 false
 */
void UBMSaveGameSubsystem::AutoSave()
{
    // 发送保存开始通知
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        EventBus->EmitNotify(NSLOCTEXT("BMSave", "Saving", "Saving..."));
    }

    bool bSuccess = SaveGame(AUTO_SAVE_SLOT);
    
    if (bSuccess)
    {
        UE_LOG(LogBMSave, Log, TEXT("Successfully saved game to slot %d"), AUTO_SAVE_SLOT);
    }
}

/**
 * 保存游戏到指定槽位
 * 
 * 保存游戏到指定槽位
 * 
 * @param Slot 存档槽位编号
 * @return 保存成功返回 true，失败返回 false
 */
bool UBMSaveGameSubsystem::SaveGame(int32 Slot)
{
    // 检查槽位编号是否有效
    if (Slot < 0)
    {
        UE_LOG(LogBMSave, Error, TEXT("SaveGame Failed: Invalid slot number %d"), Slot);
        return false;
    }

    // 获取玩家角色
    ABMPlayerCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        UE_LOG(LogBMSave, Error, TEXT("SaveGame Failed: Player Character not found."));
        return false;
    }

    // 创建 SaveData 对象
    UBMSaveData* SaveDataInstance = Cast<UBMSaveData>(
        UGameplayStatics::CreateSaveGameObject(UBMSaveData::StaticClass())
    );

    // 检查 SaveData 对象是否创建成功
    if (!SaveDataInstance)
    {
        UE_LOG(LogBMSave, Error, TEXT("SaveGame Failed: Failed to create SaveData object."));
        return false;
    }

    // 写入存档数据
    SaveDataInstance->SaveSlotName = GetSlotName(Slot);
    WriteSaveData(SaveDataInstance, Player);

    // 验证存档数据有效性
    if (!ValidateSaveData(SaveDataInstance))
    {
        UE_LOG(LogBMSave, Error, TEXT("SaveGame Failed: SaveData validation failed."));
        return false;
    }

    // 发送保存开始通知（如果不是自动保存）
    if (Slot != AUTO_SAVE_SLOT)
    {
        if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "Saving", "Saving..."));
        }
    }

    // 保存存档数据到磁盘
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveDataInstance, SaveDataInstance->SaveSlotName, 0);

    // 检查保存是否成功并发送通知
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        if (bSuccess)
        {
            UE_LOG(LogBMSave, Log, TEXT("Successfully saved game to slot %d"), Slot);
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "Saved", "Game saved successfully"));
        }
        else
        {
            UE_LOG(LogBMSave, Error, TEXT("Failed to save game to slot %d"), Slot);
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "SaveFailed", "Failed to save game"));
        }
    }

    return bSuccess;
}

/**
 * 从指定槽位加载游戏
 * 
 * 从指定槽位加载游戏
 * 
 * @param Slot 存档槽位编号
 * @param bRestorePosition 是否恢复玩家位置（默认 true）
 * @return 加载成功返回 true，失败返回 false
 */
bool UBMSaveGameSubsystem::LoadGame(int32 Slot, bool bRestorePosition)
{
    if (Slot < 0)
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Invalid slot number %d"), Slot);
        return false;
    }

    // 设置当前存档槽位索引
    CurrentSlotIndex = Slot;
    FString SlotName = GetSlotName(Slot);
    
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogBMSave, Warning, TEXT("LoadGame Failed: Slot %d does not exist."), Slot);
        if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "SlotNotExist", "Save slot does not exist"));
        }
        return false;
    }

    // 从磁盘读取
    UBMSaveData* LoadedData = Cast<UBMSaveData>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!LoadedData)
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Failed to load SaveData from slot %d"), Slot);
        if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "LoadFailed", "Failed to load game"));
        }
        return false;
    }

    // 验证数据有效性
    if (!ValidateSaveData(LoadedData))
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Loaded SaveData validation failed."));
        if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "ValidationFailed", "Save data is corrupted"));
        }
        return false;
    }

    ABMPlayerCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Player Character not found."));
        return false;
    }

    // 应用数据
    ApplySaveData(LoadedData, Player, bRestorePosition);
    
    // 发送加载成功通知
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        EventBus->EmitNotify(NSLOCTEXT("BMSave", "Loaded", "Game loaded successfully"));
    }
    
    UE_LOG(LogBMSave, Log, TEXT("Successfully loaded game from slot %d (RestorePosition: %s)"), 
        Slot, bRestorePosition ? TEXT("true") : TEXT("false"));
    return true;
}

/**
 * 获取存档元信息
 * 
 * @param Slot 存档槽位编号
 * @param OutMeta 输出的元信息结构
 * @return 成功获取返回 true，失败返回 false
 */
bool UBMSaveGameSubsystem::GetSaveMeta(int32 Slot, FBMSaveMeta& OutMeta)
{
    if (Slot < 0)
    {
        UE_LOG(LogBMSave, Error, TEXT("GetSaveMeta Failed: Invalid slot number %d"), Slot);
        return false;
    }

    if (!DoesSaveExist(Slot))
    {
        UE_LOG(LogBMSave, Warning, TEXT("GetSaveMeta Failed: Slot %d does not exist"), Slot);
        return false;
    }

    FString SlotName = GetSlotName(Slot);
    UBMSaveData* LoadedData = Cast<UBMSaveData>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!LoadedData)
    {
        UE_LOG(LogBMSave, Error, TEXT("GetSaveMeta Failed: Failed to load SaveData from slot %d"), Slot);
        return false;
    }

    OutMeta = LoadedData->GetSaveMeta();
    return true;
}

bool UBMSaveGameSubsystem::DeleteSave(int32 Slot)
{
    if (Slot < 0)
    {
        UE_LOG(LogBMSave, Error, TEXT("DeleteSave Failed: Invalid slot number %d"), Slot);
        if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "DeleteFailed", "Failed to delete save"));
        }
        return false;
    }

    FString SlotName = GetSlotName(Slot);
    
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogBMSave, Warning, TEXT("DeleteSave: Slot %d does not exist."), Slot);
        return false;
    }

    bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, 0);
    
    // 发送删除结果通知
    if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
    {
        if (bSuccess)
        {
            UE_LOG(LogBMSave, Log, TEXT("Successfully deleted save from slot %d"), Slot);
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "Deleted", "Save deleted successfully"));
        }
        else
        {
            UE_LOG(LogBMSave, Error, TEXT("Failed to delete save from slot %d"), Slot);
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "DeleteFailed", "Failed to delete save"));
        }
    }

    return bSuccess;
}

TArray<FBMSaveSlotInfo> UBMSaveGameSubsystem::GetAllSaveSlots(int32 MaxSlots)
{
    TArray<FBMSaveSlotInfo> SaveSlots;
    SaveSlots.Reserve(MaxSlots);

    for (int32 Slot = 0; Slot < MaxSlots; ++Slot)
    {
        FBMSaveSlotInfo SlotInfo;
        SlotInfo.SlotNumber = Slot;
        SlotInfo.bExists = DoesSaveExist(Slot);

        if (SlotInfo.bExists)
        {
            // 获取存档元信息
            GetSaveMeta(Slot, SlotInfo.Meta);
        }

        SaveSlots.Add(SlotInfo);
    }

    return SaveSlots;
}

UBMEventBusSubsystem* UBMSaveGameSubsystem::GetEventBusSubsystem() const
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        return GameInstance->GetSubsystem<UBMEventBusSubsystem>();
    }
    return nullptr;
}

/**
 * 验证存档数据有效性
 * 
 * 验证存档数据有效性
 * 
 * @param SaveData 要验证的存档数据
 * @return 数据有效返回 true，否则返回 false
 */
bool UBMSaveGameSubsystem::ValidateSaveData(const UBMSaveData* SaveData) const
{
    if (!SaveData)
    {
        return false;
    }

    return SaveData->IsValid();
}

// === 数据收集逻辑 ===
/**
 * 将游戏世界数据写入 SaveData 对象
 * 
 * @param SaveData 要写入的存档数据对象
 * @param Player 玩家角色指针
 */
void UBMSaveGameSubsystem::WriteSaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player)
{
    if (!SaveData || !Player)
    {
        UE_LOG(LogBMSave, Error, TEXT("WriteSaveData: Invalid parameters"));
        return;
    }

    // 1. 基础信息和位置
    SaveData->Timestamp = FDateTime::Now();
    SaveData->Location = Player->GetActorLocation();
    SaveData->Rotation = Player->GetActorRotation();
    
    // 获取当前地图名称
    if (UWorld* World = GetWorld())
    {
        FString MapName = UGameplayStatics::GetCurrentLevelName(World, true);
        if (!MapName.IsEmpty())
        {
            SaveData->MapName = FName(*MapName);
        }
    }

    // 2. 收集完整 Stats
    if (UBMStatsComponent* Stats = Player->GetStats())
    {
        const FBMStatBlock& StatBlock = Stats->GetStatBlock();
        
        SaveData->MaxHP = StatBlock.MaxHP;
        SaveData->HP = StatBlock.HP;
        SaveData->MaxMP = StatBlock.MaxMP;
        SaveData->MP = StatBlock.MP;
        SaveData->MaxStamina = StatBlock.MaxStamina;
        SaveData->Stamina = StatBlock.Stamina;
        SaveData->Attack = StatBlock.Attack;
        SaveData->Defense = StatBlock.Defense;
        SaveData->MoveSpeed = StatBlock.MoveSpeed;
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("WriteSaveData: Stats component not found"));
    }

    // 3. 收集 Experience
    if (UBMExperienceComponent* ExpComp = Player->GetComponentByClass<UBMExperienceComponent>())
    {
        SaveData->PlayerLevel = ExpComp->GetLevel();
        SaveData->CurrentXP = ExpComp->GetCurrentXP();
        SaveData->SkillPoints = ExpComp->GetSkillPoints();
        SaveData->AttributePoints = ExpComp->GetAttributePoints();
        UE_LOG(LogBMSave, Verbose, TEXT("WriteSaveData: Saved experience data - Level: %d, XP: %f, SkillPoints: %d, AttributePoints: %d"), 
            SaveData->PlayerLevel, SaveData->CurrentXP, SaveData->SkillPoints, SaveData->AttributePoints);
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("WriteSaveData: Experience component not found"));
    }

    // 4. 收集 Inventory
    if (UBMInventoryComponent* InvComp = Player->GetComponentByClass<UBMInventoryComponent>())
    {
        SaveData->InventoryItems.Empty();
        
        // 使用公共接口获取所有物品，转换为 SaveData 格式
        const TMap<FName, int32>& Items = InvComp->GetAllItems();
        for (const auto& ItemPair : Items)
        {
            FMBInventoryItemSaveData ItemSaveData;
            ItemSaveData.ItemID = ItemPair.Key;
            ItemSaveData.Quantity = ItemPair.Value;
            SaveData->InventoryItems.Add(ItemSaveData);
        }
        
        // 保存货币
        SaveData->Currency = InvComp->GetCurrency();
        
        UE_LOG(LogBMSave, Verbose, TEXT("WriteSaveData: Saved inventory - %d item types, Currency: %d"), 
            SaveData->InventoryItems.Num(), SaveData->Currency);
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("WriteSaveData: Inventory component not found"));
    }
}

// === 数据恢复逻辑 ===
/**
 * 将 SaveData 对象的数据应用回游戏世界
 * 
 * @param SaveData 要应用的存档数据对象
 * @param Player 玩家角色指针
 * @param bRestorePosition 是否恢复位置（默认 true）
 */
void UBMSaveGameSubsystem::ApplySaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player, bool bRestorePosition)
{
    if (!SaveData || !Player)
    {
        UE_LOG(LogBMSave, Error, TEXT("ApplySaveData: Invalid parameters"));
        return;
    }

    // 1. 恢复位置和旋转（可选）
    if (bRestorePosition)
    {
        Player->SetActorLocation(SaveData->Location, false, nullptr, ETeleportType::TeleportPhysics);
        Player->SetActorRotation(SaveData->Rotation, ETeleportType::TeleportPhysics);
        UE_LOG(LogBMSave, Log, TEXT("ApplySaveData: Restored position (%.1f, %.1f, %.1f)"), 
            SaveData->Location.X, SaveData->Location.Y, SaveData->Location.Z);
    }
    else
    {
        UE_LOG(LogBMSave, Log, TEXT("ApplySaveData: Skipped position restore (using PlayerStart)"));
    }

    // 2. 恢复完整 Stats
    if (UBMStatsComponent* Stats = Player->GetStats())
    {
        FBMStatBlock& StatBlock = Stats->GetStatBlockMutable();
        
        StatBlock.MaxHP = SaveData->MaxHP;
        StatBlock.HP = FMath::Clamp(SaveData->HP, 0.0f, SaveData->MaxHP);
        StatBlock.MaxMP = SaveData->MaxMP;
        StatBlock.MP = FMath::Clamp(SaveData->MP, 0.0f, SaveData->MaxMP);
        StatBlock.MaxStamina = SaveData->MaxStamina;
        StatBlock.Stamina = FMath::Clamp(SaveData->Stamina, 0.0f, SaveData->MaxStamina);
        StatBlock.Attack = SaveData->Attack;
        StatBlock.Defense = SaveData->Defense;
        StatBlock.MoveSpeed = SaveData->MoveSpeed;
        
        // TODO: 如果需要刷新 UI，可以在这里触发事件
        // Stats->OnHPChanged.Broadcast(StatBlock.HP);
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("ApplySaveData: Stats component not found"));
    }

    // 3. 恢复 Experience
    if (UBMExperienceComponent* ExpComp = Player->GetComponentByClass<UBMExperienceComponent>())
    {
        // 设置等级（会自动应用成长数据）
        ExpComp->SetLevel(SaveData->PlayerLevel, true);
        
        // 设置经验值（会自动检查升级）
        ExpComp->SetCurrentXP(SaveData->CurrentXP);
        
        // 设置技能点数和属性点数
        ExpComp->SetSkillPoints(SaveData->SkillPoints);
        ExpComp->SetAttributePoints(SaveData->AttributePoints);
        
        UE_LOG(LogBMSave, Log, TEXT("ApplySaveData: Restored experience data - Level: %d, XP: %f, SkillPoints: %d, AttributePoints: %d"), 
            SaveData->PlayerLevel, SaveData->CurrentXP, SaveData->SkillPoints, SaveData->AttributePoints);
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("ApplySaveData: Experience component not found"));
    }

    // 4. 恢复 Inventory
    if (UBMInventoryComponent* InvComp = Player->GetComponentByClass<UBMInventoryComponent>())
    {
        // 清空当前背包
        InvComp->ClearInventory();
        
        // 恢复所有物品
        for (const FMBInventoryItemSaveData& ItemData : SaveData->InventoryItems)
        {
            // 验证 ItemID 和 Quantity 的有效性
            if (ItemData.ItemID != NAME_None && ItemData.Quantity > 0)
            {
                // 使用 AddItem 方法添加物品（会自动处理堆叠限制等逻辑）
                if (!InvComp->AddItem(ItemData.ItemID, ItemData.Quantity))
                {
                    UE_LOG(LogBMSave, Warning, TEXT("ApplySaveData: Failed to restore item %s (Quantity: %d)"), 
                        *ItemData.ItemID.ToString(), ItemData.Quantity);
                }
            }
            else
            {
                UE_LOG(LogBMSave, Warning, TEXT("ApplySaveData: Invalid inventory item data - ItemID: %s, Quantity: %d"), 
                    *ItemData.ItemID.ToString(), ItemData.Quantity);
            }
        }
        
        // 恢复货币
        // 使用存档系统专用接口直接设置货币值
        InvComp->SetCurrencyDirect(SaveData->Currency);
        
        UE_LOG(LogBMSave, Log, TEXT("ApplySaveData: Restored inventory - %d item types, Currency: %d"), 
            SaveData->InventoryItems.Num(), SaveData->Currency);
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("ApplySaveData: Inventory component not found"));   
    }
}

bool UBMSaveGameSubsystem::SaveCurrentGame()
{
    return SaveGame(CurrentSlotIndex);
}

int32 UBMSaveGameSubsystem::GetNextAvailableSlot(){
    for (int32 Slot = 0; Slot < MAX_SAVE_SLOTS; ++Slot)
    {
        if (!DoesSaveExist(Slot))
        {
            return Slot;
        }
    }
    return MAX_SAVE_SLOTS;
}

void UBMSaveGameSubsystem::StartNewGame()
{
    CurrentSlotIndex = GetNextAvailableSlot();
    if (CurrentSlotIndex != MAX_SAVE_SLOTS)
    {
        SaveGame(CurrentSlotIndex);
    }
    else
    {
        UE_LOG(LogBMSave, Error, TEXT("StartNewGame Failed: No available slot found"));
        if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "StartNewGameFailed", "Failed to start new game"));
        }
    }
}

// ===== 手动存档便捷方法 =====

bool UBMSaveGameSubsystem::SaveGameManual()
{
    UE_LOG(LogBMSave, Log, TEXT("Manual save requested (Slot 1)"));
    return SaveGame(MANUAL_SAVE_SLOT);
}

bool UBMSaveGameSubsystem::LoadGameManual()
{
    UE_LOG(LogBMSave, Log, TEXT("Manual load requested (Slot 1)"));
    
    if (!HasManualSave())
    {
        UE_LOG(LogBMSave, Warning, TEXT("LoadGameManual: No manual save found"));
        if (UBMEventBusSubsystem* EventBus = GetEventBusSubsystem())
        {
            EventBus->EmitNotify(NSLOCTEXT("BMSave", "NoManualSave", "No save file found"));
        }
        return false;
    }
    
    // 手动读档恢复位置（回到存档点）
    return LoadGame(MANUAL_SAVE_SLOT, true);
}

bool UBMSaveGameSubsystem::HasManualSave() const
{
    return DoesSaveExist(MANUAL_SAVE_SLOT);
}