#include "System/Save/BMSaveGameSubsystem.h"

#include "System/Save/BMSaveData.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMExperienceComponent.h"

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
bool UBMSaveGameSubsystem::DoesSaveExist(int32 Slot)
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
    SaveGame(AUTO_SAVE_SLOT);
    UE_LOG(LogBMSave, Log, TEXT("Successfully saved game to slot %d"), AUTO_SAVE_SLOT);
    // TODO: 在此处调用 UI Subsystem 显示 "Saving..." 通知
    // UBMUIManager* UIManager = GetUIManager();
    // if (UIManager)
    // {
    //     UIManager->ShowSavingNotification();
    // }
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

    // 保存存档数据到磁盘
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveDataInstance, SaveDataInstance->SaveSlotName, 0);

    // 检查保存是否成功
    if (bSuccess)
    {
        UE_LOG(LogBMSave, Log, TEXT("Successfully saved game to slot %d"), Slot);
        // TODO: 在此处调用 UI Subsystem 显示 "Saved" 通知
        // UBMUIManager* UIManager = GetUIManager();
        // if (UIManager)
        // {
        //     UIManager->ShowSavedNotification();
        // }
    }
    else
    {
        UE_LOG(LogBMSave, Error, TEXT("Failed to save game to slot %d"), Slot);
        // TODO: 在此处调用 UI Subsystem 显示 "Failed to save" 通知
        // UBMUIManager* UIManager = GetUIManager();
        // if (UIManager)
        // {
        //     UIManager->ShowFailedToSaveNotification();
        // }
    }

    return bSuccess;
}

/**
 * 从指定槽位加载游戏
 * 
 * 从指定槽位加载游戏
 * 
 * @param Slot 存档槽位编号
 * @return 加载成功返回 true，失败返回 false
 */
bool UBMSaveGameSubsystem::LoadGame(int32 Slot)
{
    if (Slot < 0)
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Invalid slot number %d"), Slot);
        return false;
    }

    FString SlotName = GetSlotName(Slot);
    
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogBMSave, Warning, TEXT("LoadGame Failed: Slot %d does not exist."), Slot);
        // TODO: 在此处调用 UI Subsystem 显示 "Slot does not exist" 通知
        // UBMUIManager* UIManager = GetUIManager();
        // if (UIManager)
        // {
        //     UIManager->ShowSlotDoesNotExistNotification();
        // }
        return false;
    }

    // 从磁盘读取
    UBMSaveData* LoadedData = Cast<UBMSaveData>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!LoadedData)
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Failed to load SaveData from slot %d"), Slot);
        // TODO: 在此处调用 UI Subsystem 显示 "Failed to load" 通知
        // UBMUIManager* UIManager = GetUIManager();
        // if (UIManager)
        // {
        //     UIManager->ShowFailedToLoadNotification();
        // }
        return false;
    }

    // 验证数据有效性
    if (!ValidateSaveData(LoadedData))
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Loaded SaveData validation failed."));
        // TODO: 在此处调用 UI Subsystem 显示 "Validation failed" 通知
        // UBMUIManager* UIManager = GetUIManager();
        // if (UIManager)
        // {
        //     UIManager->ShowValidationFailedNotification();
        // }
        return false;
    }

    ABMPlayerCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        UE_LOG(LogBMSave, Error, TEXT("LoadGame Failed: Player Character not found."));
        return false;
    }

    // 应用数据
    ApplySaveData(LoadedData, Player);
    // TODO: 在此处调用 UI Subsystem 显示 "Loaded" 通知
    // UBMUIManager* UIManager = GetUIManager();
    // if (UIManager)
    // {
    //     UIManager->ShowLoadedNotification();
    // }
    UE_LOG(LogBMSave, Log, TEXT("Successfully loaded game from slot %d"), Slot);
    return true;
}

/**
 * 获取存档元信息（用于UI显示）
 * 
 * 获取存档元信息（用于UI显示）
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
        // TODO: 在此处调用 UI Subsystem 显示 "Failed to delete save" 通知
        // UBMUIManager* UIManager = GetUIManager();
        // if (UIManager)
        // {
        //     UIManager->ShowFailedToDeleteSaveNotification();
        // }
        return false;
    }

    FString SlotName = GetSlotName(Slot);
    
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogBMSave, Warning, TEXT("DeleteSave: Slot %d does not exist."), Slot);
        return false;
    }

    bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, 0);
    
    if (bSuccess)
    {
        UE_LOG(LogBMSave, Log, TEXT("Successfully deleted save from slot %d"), Slot);
    }
    else
    {
        UE_LOG(LogBMSave, Error, TEXT("Failed to delete save from slot %d"), Slot);
    }

    return bSuccess;
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

    // 2. 收集完整 Stats（使用正确的访问器 GetStats()）
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

    // 3. 收集 Experience（使用 GetComponentByClass 查找组件）
    if (UBMExperienceComponent* ExpComp = Player->GetComponentByClass<UBMExperienceComponent>())
    {
        // 注意：这些属性可能需要在 ExperienceComponent 中实际实现
        // TODO: 需要根据组件实际接口调整
        // SaveData->PlayerLevel = ExpComp->Level;
        // SaveData->CurrentXP = ExpComp->CurrentXP;
        // SaveData->SkillPoints = ExpComp->SkillPoints;
        UE_LOG(LogBMSave, Verbose, TEXT("WriteSaveData: Experience component found (implementation pending)"));
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("WriteSaveData: Experience component not found"));
    }

    // 4. 收集 Inventory（使用 GetComponentByClass 查找组件）
    if (UBMInventoryComponent* InvComp = Player->GetComponentByClass<UBMInventoryComponent>())
    {
        SaveData->InventoryItems.Empty();
        
        // 注意：这里假设 InventoryComponent 有 Items 成员（TMap<FName, int32>）
        // TODO: 需要根据组件实际接口调整
        // 实际使用时需要根据组件实际接口调整
        // 如果组件有 GetItems() 方法，应该使用它
        // const TMap<FName, int32>& Items = InvComp->GetItems();
        // for (const auto& Pair : Items)
        // {
        //     FMBInventoryItemSaveData NewItem;
        //     NewItem.ItemID = Pair.Key;
        //     NewItem.Quantity = Pair.Value;
        //     SaveData->InventoryItems.Add(NewItem);
        // }
        UE_LOG(LogBMSave, Verbose, TEXT("WriteSaveData: Inventory component found (implementation pending)"));
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
 * 将 SaveData 对象的数据应用回游戏世界
 * 
 * @param SaveData 要应用的存档数据对象
 * @param Player 玩家角色指针
 */
void UBMSaveGameSubsystem::ApplySaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player)
{
    if (!SaveData || !Player)
    {
        UE_LOG(LogBMSave, Error, TEXT("ApplySaveData: Invalid parameters"));
        return;
    }

    // 1. 恢复位置和旋转
    // TODO: 如果有 NavMesh 或碰撞，直接 SetLocation 可能会导致穿模，通常建议加检测
    Player->SetActorLocation(SaveData->Location, false, nullptr, ETeleportType::TeleportPhysics);
    Player->SetActorRotation(SaveData->Rotation, ETeleportType::TeleportPhysics);

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
        // TODO: 这些属性可能需要在 ExperienceComponent 中实际实现
        // 实际使用时需要根据组件实际接口调整
        // ExpComp->Level = SaveData->PlayerLevel;
        // ExpComp->CurrentXP = SaveData->CurrentXP;
        // ExpComp->SkillPoints = SaveData->SkillPoints;
        // ExpComp->CheckLevelUp(); // 视逻辑决定是否检查
        UE_LOG(LogBMSave, Verbose, TEXT("ApplySaveData: Experience component found (implementation pending)"));
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("ApplySaveData: Experience component not found"));
    }

    // 4. 恢复 Inventory
    if (UBMInventoryComponent* InvComp = Player->GetComponentByClass<UBMInventoryComponent>())
    {
        // TODO: 这里假设 InventoryComponent 有 Items 成员（TMap<FName, int32>）
        // TODO: 如果组件有 ClearItems() 和 AddItem() 方法，应该使用它们
        // InvComp->ClearItems();
        // for (const FMBInventoryItemSaveData& ItemData : SaveData->InventoryItems)
        // {
        //     InvComp->AddItem(ItemData.ItemID, ItemData.Quantity);
        // }
        UE_LOG(LogBMSave, Verbose, TEXT("ApplySaveData: Inventory component found (implementation pending)"));
    }
    else
    {
        UE_LOG(LogBMSave, Warning, TEXT("ApplySaveData: Inventory component not found"));
    }
}