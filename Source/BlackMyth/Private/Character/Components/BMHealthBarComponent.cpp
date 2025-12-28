#include "Character/Components/BMHealthBarComponent.h"

#include "Character/BMCharacterBase.h"
#include "Character/Components/BMStatsComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

/*
 * @brief Constructor of the UBMHealthBarComponent class, it initializes the component and sets the auto activate and hidden in game properties
 * @param USceneComponent The parent class
 */
UBMHealthBarComponent::UBMHealthBarComponent() 
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;
    bHiddenInGame = false;
}

/*
 * @brief Begin play, it applies the vertical offset and observes the character, 
 * if the observed character is not valid, it casts the owner to ABMCharacterBase and observes the character
 */
void UBMHealthBarComponent::BeginPlay()
{
    Super::BeginPlay();
    ApplyVerticalOffset();

    if (!ObservedCharacter.IsValid())
    {
        if (ABMCharacterBase* CharacterOwner = Cast<ABMCharacterBase>(GetOwner()))
        {
            ObserveCharacter(CharacterOwner);
        }
    }

    RefreshFromStats();
}

/*
 * @brief End play, it unbinds the character and calls the super end play
 * @param EndPlayReason The reason for the end play
 */
void UBMHealthBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindFromCharacter();
    Super::EndPlay(EndPlayReason);
}

/*
 * @brief Tick component, it faces the camera if the bFaceCamera property is true
 * @param DeltaTime The delta time
 * @param TickType The tick type
 * @param ThisTickFunction The tick function
 */
void UBMHealthBarComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bFaceCamera)
    {
        FaceCamera();
    }
}

/*
 * @brief Observe character, it observes the character if the observed character is not the same as the input character
 * @param InCharacter The character to observe
 */
void UBMHealthBarComponent::ObserveCharacter(ABMCharacterBase* InCharacter)
{
    if (ObservedCharacter.Get() == InCharacter)
    {
        return;
    }

    UnbindFromCharacter();
    BindToCharacter(InCharacter);
}

/*
 * @brief Set vertical offset, it sets the vertical offset and applies the vertical offset
 * @param NewOffset The new offset
 */
void UBMHealthBarComponent::SetVerticalOffset(float NewOffset)
{
    VerticalOffset = NewOffset;
    ApplyVerticalOffset();
}

/*
 * @brief Refresh from stats, it refreshes the health bar from the stats component
 */
void UBMHealthBarComponent::RefreshFromStats()
{
    UBMStatsComponent* StatsComponent = ObservedStats.Get();
    if (!StatsComponent)
    {
        SetVisibility(false, true);
        return;
    }

    const FBMStatBlock& Block = StatsComponent->GetStatBlock();
    const float MaxHP = FMath::Max(Block.MaxHP, 1.f);
    const float CurrentHP = FMath::Clamp(Block.HP, 0.f, MaxHP);

    if (bHideWhenFull && FMath::IsNearlyEqual(CurrentHP, MaxHP))
    {
        SetVisibility(false, true);
    }
    else
    {
        SetVisibility(true, true);
    }

    UpdateHealthDisplay(CurrentHP, MaxHP);
}

/*
 * @brief Bind to character, it binds the character to the health bar and refreshes the health bar from the stats component
 * @param Character The character to bind to
 */
void UBMHealthBarComponent::BindToCharacter(ABMCharacterBase* Character)
{
    if (!Character)
    {
        return;
    }

    ObservedCharacter = Character;
    ObservedStats = Character->GetStats();

    DamagedHandle = Character->OnCharacterDamaged.AddUObject(this, &UBMHealthBarComponent::HandleCharacterDamaged);
    DeathHandle = Character->OnCharacterDied.AddUObject(this, &UBMHealthBarComponent::HandleCharacterDied);

    RefreshFromStats();
}

/*
 * @brief Unbind from character, it unbinds the character from the health bar and resets the observed character and stats
 */
void UBMHealthBarComponent::UnbindFromCharacter()
{
    if (!ObservedCharacter.IsValid())
    {
        return;
    }

    if (DamagedHandle.IsValid())
    {
        ObservedCharacter->OnCharacterDamaged.Remove(DamagedHandle);
        DamagedHandle.Reset();
    }

    if (DeathHandle.IsValid())
    {
        ObservedCharacter->OnCharacterDied.Remove(DeathHandle);
        DeathHandle.Reset();
    }

    ObservedCharacter.Reset();
    ObservedStats.Reset();
}

/*
 * @brief Handle character damaged, it refreshes the health bar from the stats component if the victim is the observed character
 * @param Victim The victim
 * @param Info The damage info
 */
void UBMHealthBarComponent::HandleCharacterDamaged(ABMCharacterBase* Victim, const FBMDamageInfo& Info)
{
    if (Victim == ObservedCharacter.Get())
    {
        RefreshFromStats();
    }
}

/*
 * @brief Handle character died, it refreshes the health bar from the stats component and sets the visibility to false if the victim is the observed character
 * @param Victim The victim
 * @param Info The damage info
 */
void UBMHealthBarComponent::HandleCharacterDied(ABMCharacterBase* Victim, const FBMDamageInfo& Info)
{
    if (Victim == ObservedCharacter.Get())
    {
        RefreshFromStats();
        SetVisibility(false, true);
    }
}

/*
 * @brief Face camera, it faces the camera if the bFaceCamera property is true
 */
void UBMHealthBarComponent::FaceCamera()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(World, 0);
    if (!Camera)
    {
        return;
    }

    FVector Direction = Camera->GetCameraLocation() - GetComponentLocation();
    Direction.Z = 0.f;

    if (!Direction.IsNearlyZero())
    {
        const FRotator LookRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        SetWorldRotation(LookRotation);
    }
}

/*
 * @brief Apply vertical offset, it applies the vertical offset to the health bar
 */
void UBMHealthBarComponent::ApplyVerticalOffset()
{
    SetRelativeLocation(FVector(0.f, 0.f, VerticalOffset));
}

/*
 * @brief Constructor of the UBMEnemyHealthBarComponent class, it initializes the component and sets the auto activate and hidden in game properties
 */
UBMEnemyHealthBarComponent::UBMEnemyHealthBarComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bHideWhenFull = false;
    bFaceCamera = true;


}

/*
 * @brief On register, it registers the background and foreground text components
 */
void UBMEnemyHealthBarComponent::OnRegister()
{
    Super::OnRegister();


    if (!BackgroundTextComponent)
    {
        BackgroundTextComponent = NewObject<UTextRenderComponent>(this, TEXT("BackgroundText"));
        if (BackgroundTextComponent)
        {
            BackgroundTextComponent->SetupAttachment(this);
            BackgroundTextComponent->RegisterComponent();
        }
    }


    if (!ForegroundTextComponent)
    {
        ForegroundTextComponent = NewObject<UTextRenderComponent>(this, TEXT("ForegroundText"));
        if (ForegroundTextComponent)
        {
            ForegroundTextComponent->SetupAttachment(this);
            ForegroundTextComponent->RegisterComponent();
        }
    }


    if (BackgroundTextComponent)
    {
        BackgroundTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
        BackgroundTextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
        BackgroundTextComponent->SetTextRenderColor(BackgroundColor);
        BackgroundTextComponent->SetWorldSize(FontWorldSize);
        BackgroundTextComponent->SetText(FText::FromString(BuildBackgroundBarString()));
        BackgroundTextComponent->SetHiddenInGame(false);
        BackgroundTextComponent->SetTranslucentSortPriority(4);

        BackgroundTextComponent->SetRelativeLocation(FVector(-0.5f, 0.f, 0.f));
    }


    if (ForegroundTextComponent)
    {
        ForegroundTextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
        ForegroundTextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
        ForegroundTextComponent->SetTextRenderColor(HealthColor);
        ForegroundTextComponent->SetWorldSize(FontWorldSize);
        ForegroundTextComponent->SetText(FText::FromString(BuildFilledBarString(SegmentCount)));
        ForegroundTextComponent->SetHiddenInGame(false);
        ForegroundTextComponent->SetTranslucentSortPriority(5);
    }
}

/*
 * @brief Update health display, it updates the health display based on the current and max HP
 * @param CurrentHP The current HP
 * @param MaxHP The max HP
 */
void UBMEnemyHealthBarComponent::UpdateHealthDisplay(float CurrentHP, float MaxHP)
{
    const float Percent = (MaxHP > 0.f) ? (CurrentHP / MaxHP) : 0.f;
    const int32 FilledSegments = FMath::Clamp(FMath::RoundToInt(Percent * SegmentCount), 0, SegmentCount);


    if (ForegroundTextComponent)
    {
        ForegroundTextComponent->SetText(FText::FromString(BuildFilledBarString(FilledSegments)));
    }


}

/*
 * @brief Build filled bar string, it builds the filled bar string based on the filled count
 * @param FilledCount The filled count
 * @return The filled bar string
 */
FString UBMEnemyHealthBarComponent::BuildFilledBarString(int32 FilledCount) const
{
    FString Bar;
    Bar.AppendChar(TEXT('['));
    for (int32 i = 0; i < FilledCount; ++i)
    {
        Bar.AppendChar(TEXT('|')); 
    }
    
    for (int32 i = FilledCount; i < SegmentCount; ++i)
    {
        Bar.AppendChar(TEXT(' '));
    }
    
    Bar.AppendChar(TEXT(']'));
    
    return Bar;
}

/*
 * @brief Build background bar string, it builds the background bar string
 * @return The background bar string
 */
FString UBMEnemyHealthBarComponent::BuildBackgroundBarString() const
{
    FString Bar;
    
    Bar.AppendChar(TEXT('['));
    
    for (int32 i = 0; i < SegmentCount; ++i)
    {
        Bar.AppendChar(TEXT('|')); 
    }
    
    Bar.AppendChar(TEXT(']'));
    
    return Bar;
}
