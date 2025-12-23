// Fill out your copyright notice in the Description page of Project Settings.


#include "BMGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMExperienceComponent.h"

void UBMGameInstance::CapturePlayerPersistentData(APlayerController* PC)
{
    bHasCapturedPersistentData = false;
    if (!PC) { return; }
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) { return; }

    if (UBMInventoryComponent* Inv = Pawn->FindComponentByClass<UBMInventoryComponent>())
    {
        PersistentCoins = Inv->GetCurrency();
        PersistentItems = Inv->GetAllItems();
        PersistentItemCount = PersistentItems.Num();
    }
    if (UBMExperienceComponent* XP = Pawn->FindComponentByClass<UBMExperienceComponent>())
    {
        PersistentExp = static_cast<int32>(XP->GetCurrentXP());
        PersistentLevel = XP->GetLevel();
        PersistentSkillPoints = XP->GetSkillPoints();
        PersistentAttributePoints = XP->GetAttributePoints();
    }
    bHasCapturedPersistentData = true;
}

void UBMGameInstance::RestorePlayerPersistentData(APlayerController* PC)
{
    if (!bHasCapturedPersistentData || !PC) { return; }
    APawn* Pawn = PC->GetPawn();
    if (!Pawn) { return; }

    if (UBMInventoryComponent* Inv = Pawn->FindComponentByClass<UBMInventoryComponent>())
    {
        Inv->SetCurrencyDirect(PersistentCoins);
        // Clear and restore items
        // Direct access for restore; we can call ClearInventory() then add items
        Inv->ClearInventory();
        for (const auto& KVP : PersistentItems)
        {
            Inv->AddItem(KVP.Key, KVP.Value);
        }
        Inv->OnInventoryChanged.Broadcast();
    }
    if (UBMExperienceComponent* XP = Pawn->FindComponentByClass<UBMExperienceComponent>())
    {
        XP->SetLevel(PersistentLevel, /*bApplyGrowth*/ false);
        XP->SetCurrentXP(PersistentExp, /*bCheckLevelUp*/ false);
        XP->SetSkillPoints(PersistentSkillPoints);
        XP->SetAttributePoints(PersistentAttributePoints);
    }
}

