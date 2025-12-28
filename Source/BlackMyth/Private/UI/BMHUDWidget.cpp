// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BMHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "Character/Components/BMExperienceComponent.h"
#include "Kismet/GameplayStatics.h"
#define LOCTEXT_NAMESPACE "BMHUD"
#include "Character/Components/BMStatsComponent.h"

/*
 * @brief Bind event bus, it bind event bus
 * @param EventBus The event bus
 */
void UBMHUDWidget::BindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;

    if (!HealthChangedHandle.IsValid())
    {
        HealthChangedHandle = EventBus->OnPlayerHealthChanged.AddWeakLambda(this, [this](float Normalized)
        {
            HandleHealthChanged(Normalized);
        });
    }
    if (!StaminaChangedHandle.IsValid())
    {
        StaminaChangedHandle = EventBus->OnPlayerStaminaChanged.AddWeakLambda(this, [this](float Normalized)
        {
            HandleStaminaChanged(Normalized);
        });
    }
    if (!SkillCooldownHandle.IsValid())
    {
        SkillCooldownHandle = EventBus->OnSkillCooldownChanged.AddWeakLambda(this, [this](FName SkillId, float RemainingSeconds)
        {
            HandleSkillCooldownChanged(SkillId, RemainingSeconds);
        });
    }

    // Bind level change from experience component via EventBus
    if (!LevelChangedHandle.IsValid())
    {
        LevelChangedHandle = EventBus->OnPlayerLevelUp.AddWeakLambda(this, [this](int32 OldLevel, int32 NewLevel)
        {
            HandleLevelChanged(NewLevel);
        });
    }

    // Proactively set initial level
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            if (UBMExperienceComponent* XP = Pawn->FindComponentByClass<UBMExperienceComponent>())
            {
                CachedXP = XP;
                if (!XPLevelUpHandle.IsValid())
                {
                    XPLevelUpHandle = XP->OnLevelUpNative.AddLambda([this](int32 OldLevel, int32 NewLevel)
                    {
                        HandleLevelChanged(NewLevel);
                    });
                }
                HandleLevelChanged(XP->GetLevel());
            }
        }
    }
    SyncInitialValues();
}

/*
 * @brief Unbind event bus, it unbind event bus
 * @param EventBus The event bus
 */
void UBMHUDWidget::UnbindEventBus(UBMEventBusSubsystem* EventBus)
{
    if (!EventBus) return;

    if (HealthChangedHandle.IsValid())
    {
        EventBus->OnPlayerHealthChanged.Remove(HealthChangedHandle);
        HealthChangedHandle.Reset();
    }
    if (StaminaChangedHandle.IsValid())
    {
        EventBus->OnPlayerStaminaChanged.Remove(StaminaChangedHandle);
        StaminaChangedHandle.Reset();
    }
    if (SkillCooldownHandle.IsValid())
    {
        EventBus->OnSkillCooldownChanged.Remove(SkillCooldownHandle);
        SkillCooldownHandle.Reset();
    }
    if (LevelChangedHandle.IsValid())
    {
        EventBus->OnPlayerLevelUp.Remove(LevelChangedHandle);
        LevelChangedHandle.Reset();
    }
    if (CachedXP.IsValid() && XPLevelUpHandle.IsValid())
    {
        CachedXP->OnLevelUpNative.Remove(XPLevelUpHandle);
        XPLevelUpHandle.Reset();
        CachedXP = nullptr;
    }
}

/*
 * @brief Handle level changed, it handle level changed
 * @param NewLevel The new level
 */
void UBMHUDWidget::HandleLevelChanged(int32 NewLevel)
{
    if (LevelText)
    {
        LevelText->SetText(FText::Format(FText::FromString(TEXT("Lv {0}")), FText::AsNumber(NewLevel)));
        LevelText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
}

/*
 * @brief Handle health changed, it handle health changed
 * @param Normalized The normalized
 */
void UBMHUDWidget::HandleHealthChanged(float Normalized)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
    }
}

/*
 * @brief Handle mana changed, it handle mana changed
 * @param Normalized The normalized
 */
void UBMHUDWidget::HandleManaChanged(float Normalized)
{
}

/*
 * @brief Handle stamina changed, it handle stamina changed
 * @param Normalized The normalized
 */
void UBMHUDWidget::HandleStaminaChanged(float Normalized)
{
    if (StaminaBar)
    {
        StaminaBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
    }
}

/*
 * @brief Handle skill cooldown changed, it handle skill cooldown changed
 * @param SkillId The skill id
 * @param RemainingSeconds The remaining seconds
 */
void UBMHUDWidget::HandleSkillCooldownChanged(FName SkillId, float RemainingSeconds)
{
    FSkillCooldownData& CooldownData = SkillCooldowns.FindOrAdd(SkillId);
    
    if (RemainingSeconds > 0.f)
    {
        CooldownData.CooldownEndTime = GetCurrentWorldTime() + RemainingSeconds;
        CooldownData.TotalCooldown = RemainingSeconds;
        CooldownData.bIsCoolingDown = true;
    }
    else
    {
        CooldownData.bIsCoolingDown = false;
        CooldownData.CooldownEndTime = 0.f;
    }
    
    UTextBlock* TextWidget = nullptr;
    if (SkillId == TEXT("Skill1"))
    {
        TextWidget = Skill1CooldownText;
    }
    else if (SkillId == TEXT("Skill2"))
    {
        TextWidget = Skill2CooldownText;
    }
    else if (SkillId == TEXT("Skill3"))
    {
        TextWidget = Skill3CooldownText;
    }
    
    if (TextWidget)
    {
        UpdateSingleCooldownDisplay(SkillId, CooldownData, TextWidget);
    }
}

/*
 * @brief Format cooldown text, it format cooldown text
 * @param RemainingSeconds The remaining seconds
 * @return The cooldown text
 */
FText UBMHUDWidget::FormatCooldownText(float RemainingSeconds) const
{
    const float Clamped = FMath::Max(0.f, RemainingSeconds);
    if (Clamped <= 0.05f)
    {
        return LOCTEXT("CooldownReady", "Ready");
    }

    if (Clamped < 10.f)
    {
        return FText::AsNumber(FMath::RoundToFloat(Clamped * 10.f) / 10.f);
    }

    if (Clamped < 60.f)
    {
        return FText::AsNumber(FMath::RoundToInt(Clamped));
    }

    const int32 Total = FMath::RoundToInt(Clamped);
    const int32 Minutes = Total / 60;
    const int32 Seconds = Total % 60;
    FNumberFormattingOptions Opts;
    Opts.MinimumIntegralDigits = 2;
    const FText SecText = FText::AsNumber(Seconds, &Opts);
    return FText::FromString(FString::Printf(TEXT("%d:%s"), Minutes, *SecText.ToString()));
}

/*
 * @brief Sync initial values, it sync initial values
 */
void UBMHUDWidget::SyncInitialValues()
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        HandleHealthChanged(1.f);
        HandleStaminaChanged(1.f);
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        HandleHealthChanged(1.f);
        HandleStaminaChanged(1.f);
        return;
    }

    if (UBMStatsComponent* Stats = PlayerPawn->FindComponentByClass<UBMStatsComponent>())
    {
        const FBMStatBlock& Block = Stats->GetStatBlock();
        const float HealthNormalized = Block.MaxHP > 0.f ? Block.HP / Block.MaxHP : 0.f;
        const float StaminaNormalized = Block.MaxStamina > 0.f ? Block.Stamina / Block.MaxStamina : 0.f;
        HandleHealthChanged(HealthNormalized);
        HandleStaminaChanged(StaminaNormalized);
        return;
    }

    HandleHealthChanged(1.f);
    HandleStaminaChanged(1.f);
}

/*
 * @brief Native tick, it native tick
 * @param MyGeometry The geometry
 * @param InDeltaTime The delta time
 */
void UBMHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    UpdateCooldownDisplays(InDeltaTime);
}

/*
 * @brief Update cooldown displays, it update cooldown displays
 * @param DeltaTime The delta time
 */
void UBMHUDWidget::UpdateCooldownDisplays(float DeltaTime)
{
    for (TPair<FName, FSkillCooldownData>& Pair : SkillCooldowns)
    {
        FName SkillId = Pair.Key;
        FSkillCooldownData& CooldownData = Pair.Value;
        
        if (!CooldownData.bIsCoolingDown)
        {
            continue;
        }
        
        UTextBlock* TextWidget = nullptr;
        if (SkillId == TEXT("Skill1"))
        {
            TextWidget = Skill1CooldownText;
        }
        else if (SkillId == TEXT("Skill2"))
        {
            TextWidget = Skill2CooldownText;
        }
        else if (SkillId == TEXT("Skill3"))
        {
            TextWidget = Skill3CooldownText;
        }
        
        if (TextWidget)
        {
            UpdateSingleCooldownDisplay(SkillId, CooldownData, TextWidget);
        }
    }
}

/*
 * @brief Update single cooldown display, it update single cooldown display
 * @param SkillId The skill id
 * @param CooldownData The cooldown data
 * @param TextWidget The text widget
 */
void UBMHUDWidget::UpdateSingleCooldownDisplay(FName SkillId, FSkillCooldownData& CooldownData, UTextBlock* TextWidget)
{
    if (!TextWidget || !CooldownData.bIsCoolingDown)
    {
        return;
    }
    
    const float CurrentTime = GetCurrentWorldTime();
    const float RemainingSeconds = FMath::Max(0.f, CooldownData.CooldownEndTime - CurrentTime);
    
    if (RemainingSeconds <= 0.05f)
    {
        CooldownData.bIsCoolingDown = false;
        TextWidget->SetText(FormatCooldownText(0.f));
        TextWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        return;
    }
    
    TextWidget->SetText(FormatCooldownText(RemainingSeconds));
    TextWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

/*
 * @brief Get current world time, it get current world time
 * @return The current world time
 */
float UBMHUDWidget::GetCurrentWorldTime() const
{
    const UWorld* World = GetWorld();
    return World ? World->GetTimeSeconds() : 0.f;
}

#undef LOCTEXT_NAMESPACE

