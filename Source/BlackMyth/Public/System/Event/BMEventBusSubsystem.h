// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Delegates/DelegateCombinations.h"
#include "BMEventBusSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMEventBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    // Basic HUD-related events
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerHealthChanged, float /*Normalized*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerManaChanged, float /*Normalized*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStaminaChanged, float /*Normalized*/);
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillCooldownChanged, FName /*SkillId*/, float /*RemainingSeconds*/);

    // Boss bar
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBossPhaseChanged, int32 /*Phase*/, FText /*PhaseHint*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossHealthChanged, float /*Normalized*/);

    // Notifications
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnNotifyMessage, FText /*Message*/);

    // Lifecycle
    DECLARE_MULTICAST_DELEGATE(FOnPlayerDied);
    // Experience/Level events
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelUp, int32 /*OldLevel*/, int32 /*NewLevel*/);
    DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPlayerXPChanged, float /*CurrentXP*/, float /*MaxXP*/, float /*Percent*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerSkillPointsChanged, int32 /*NewSkillPoints*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerAttributePointsChanged, int32 /*NewAttributePoints*/);

public:
    // Dispatchers
    FOnPlayerHealthChanged OnPlayerHealthChanged;
    FOnPlayerManaChanged OnPlayerManaChanged;
    FOnPlayerStaminaChanged OnPlayerStaminaChanged;
    FOnSkillCooldownChanged OnSkillCooldownChanged;

    FOnBossPhaseChanged OnBossPhaseChanged;
    FOnBossHealthChanged OnBossHealthChanged;

    FOnNotifyMessage OnNotifyMessage;
    FOnPlayerDied OnPlayerDied;

    FOnPlayerLevelUp OnPlayerLevelUp;
    FOnPlayerXPChanged OnPlayerXPChanged;
    FOnPlayerSkillPointsChanged OnPlayerSkillPointsChanged;
    FOnPlayerAttributePointsChanged OnPlayerAttributePointsChanged;

public:
    // Convenience emitters
    void EmitPlayerHealth(float Normalized) { OnPlayerHealthChanged.Broadcast(Normalized); }
    void EmitPlayerMana(float Normalized) { OnPlayerManaChanged.Broadcast(Normalized); }
    void EmitPlayerStamina(float Normalized) { OnPlayerStaminaChanged.Broadcast(Normalized); }
    void EmitSkillCooldown(FName SkillId, float Remaining) { OnSkillCooldownChanged.Broadcast(SkillId, Remaining); }
    void EmitBossPhase(int32 Phase, const FText& Hint) { OnBossPhaseChanged.Broadcast(Phase, Hint); }
    void EmitBossHealth(float Normalized) { OnBossHealthChanged.Broadcast(Normalized); }
    void EmitNotify(const FText& Msg) { OnNotifyMessage.Broadcast(Msg); }
    void EmitPlayerDied() { OnPlayerDied.Broadcast(); }
    
    void EmitPlayerLevelUp(int32 OldLevel, int32 NewLevel) { OnPlayerLevelUp.Broadcast(OldLevel, NewLevel); }
    void EmitPlayerXPChanged(float CurrentXP, float MaxXP, float Percent) { OnPlayerXPChanged.Broadcast(CurrentXP, MaxXP, Percent); }
    void EmitPlayerSkillPointsChanged(int32 NewSkillPoints) { OnPlayerSkillPointsChanged.Broadcast(NewSkillPoints); }
    void EmitPlayerAttributePointsChanged(int32 NewAttributePoints) { OnPlayerAttributePointsChanged.Broadcast(NewAttributePoints); }
};
