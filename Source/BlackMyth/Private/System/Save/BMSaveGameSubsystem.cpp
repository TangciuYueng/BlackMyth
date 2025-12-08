
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#include "System/Save/BMSaveGameSubsystem.h"
#include "Character/BMPlayerCharacter.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMExperienceComponent.h"

FString UBMSaveGameSubsystem::GetSlotName(int32 Slot) const
{
    // Naming rule like SaveSlot_0, SaveSlot_1
    return FString::Printf(TEXT("SaveSlot_%d"), Slot);
}

ABMPlayerCharacter* UBMSaveGameSubsystem::GetPlayerCharacter() const
{
    // 获取 PlayerController 0 控制的 Pawn
    if (UWorld* World = GetWorld())
    {
        return Cast<ABMPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    }
    return nullptr;
}

/*
 * Check if a save game exists in the specified slot.
 */
bool UBMSaveGameSubsystem::DoesSaveExist(int32 Slot)
{
    return UGameplayStatics::DoesSaveGameExist(GetSlotName(Slot), 0);
}

void UBMSaveGameSubsystem::AutoSave()
{
    // 定义 0 号槽位为自动存档
    SaveGame(0);
    
    // TODO: 在此处调用 UI Subsystem 显示 "Saving..." 通知
}

bool UBMSaveGameSubsystem::SaveGame(int32 Slot)
{
    ABMPlayerCharacter* Player = GetPlayerCharacter();
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveGame Failed: Player Character not found."));
        return false;
    }

    // 创建 SaveData 实例
    UBMSaveData* SaveDataInstance = Cast<UBMSaveData>(
        UGameplayStatics::CreateSaveGameObject(UBMSaveData::StaticClass())
    );

    if (!SaveDataInstance) return false;

    // 写入数据
    SaveDataInstance->SaveSlotName = GetSlotName(Slot);
    WriteSaveData(SaveDataInstance, Player);

    // 保存到磁盘
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveDataInstance, SaveDataInstance->SaveSlotName, 0);

    UE_LOG(LogTemp, Log, TEXT("Saved Game to Slot %d: %s"), Slot, bSuccess ? TEXT("Success") : TEXT("Failed"));
    return bSuccess;
}

bool UBMSaveGameSubsystem::LoadGame(int32 Slot)
{
    FString SlotName = GetSlotName(Slot);
    
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("LoadGame Failed: Slot %d does not exist."), Slot);
        return false;
    }

    // 从磁盘读取
    UBMSaveData* LoadedData = Cast<UBMSaveData>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0)
    );

    if (!LoadedData) return false;

    ABMPlayerCharacter* Player = GetPlayerCharacter();
    if (!Player) return false;

    // 应用数据
    ApplySaveData(LoadedData, Player);

    UE_LOG(LogTemp, Log, TEXT("Loaded Game from Slot %d"), Slot);
    return true;
}

// TODO: 暂时先这样假设有以下基本接口，后续根据实际组件调整
// TODO: Aysnc Load/Save 逻辑待补充
// === 数据收集逻辑 ===
void UBMSaveGameSubsystem::WriteSaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player)
{
    if (!SaveData || !Player) return;

    // 1. 基础信息
    SaveData->Timestamp = FDateTime::Now();
    SaveData->Location = Player->GetActorLocation();

    // 2. 收集 Stats (假设你在 CharacterBase 里有 GetStatsComponent 访问器)
    if (UBMStatsComponent* Stats = Player->GetStatsComponent())
    {
        SaveData->PlayerHP = Stats->HP;
    }

    // 3. 收集 Experience
    if (UBMExperienceComponent* ExpComp = Player->GetExperienceComponent())
    {
        SaveData->PlayerLevel = ExpComp->Level;
        SaveData->CurrentXP = ExpComp->CurrentXP;
        SaveData->SkillPoints = ExpComp->SkillPoints;
    }

    // 4. 收集 Inventory (将 TMap 转换为 TArray)
    if (UBMInventoryComponent* InvComp = Player->GetInventoryComponent())
    {
        SaveData->InventoryItems.Empty();
        
        // 假设 InventoryComponent 中的 Items 是 TMap<FName, int>
        for (const auto& Pair : InvComp->Items)
        {
            FMBInventoryItemSaveData NewItem;
            NewItem.ItemID = Pair.Key;
            NewItem.Quantity = Pair.Value;
            SaveData->InventoryItems.Add(NewItem);
        }
    }
}

// === 数据恢复逻辑 ===
void UBMSaveGameSubsystem::ApplySaveData(UBMSaveData* SaveData, ABMPlayerCharacter* Player)
{
    if (!SaveData || !Player) return;

    // 1. 恢复位置
    // 注意：如果有 NavMesh 或碰撞，直接 SetLocation 可能会导致穿模，通常建议加检测
    Player->SetActorLocation(SaveData->Location, false, nullptr, ETeleportType::TeleportPhysics);

    // 2. 恢复 Stats
    if (UBMStatsComponent* Stats = Player->GetStatsComponent())
    {
        Stats->HP = SaveData->PlayerHP;
        // 如果有最大生命值逻辑，可能需要先 RecalculateMaxHP()
        // Stats->OnHPChanged.Broadcast(Stats->HP); // 如果需要刷新 UI
    }

    // 3. 恢复 Experience
    if (UBMExperienceComponent* ExpComp = Player->GetExperienceComponent())
    {
        ExpComp->Level = SaveData->PlayerLevel;
        ExpComp->CurrentXP = SaveData->CurrentXP;
        ExpComp->SkillPoints = SaveData->SkillPoints;
        // ExpComp->CheckLevelUp(); // 视逻辑决定是否检查
    }

    // 4. 恢复 Inventory (将 TArray 解析回 Component)
    if (UBMInventoryComponent* InvComp = Player->GetInventoryComponent())
    {
        // 先清空当前背包
        InvComp->Items.Empty();

        for (const FMBInventoryItemSaveData& ItemData : SaveData->InventoryItems)
        {
            // 假设 InventoryComponent 有 AddItem 接口，或者直接操作 Items Map
            // InvComp->AddItem(ItemData.ItemID, ItemData.Quantity); 
            // 如果直接操作 Map:
            InvComp->Items.Add(ItemData.ItemID, ItemData.Quantity);
        }
    }
}