#include "Character/Components/BMHealthBarComponent.h"

#include "Character/BMCharacterBase.h"
#include "Character/Components/BMStatsComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

UBMHealthBarComponent::UBMHealthBarComponent() 
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;
    bHiddenInGame = false;
}

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

void UBMHealthBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindFromCharacter();
    Super::EndPlay(EndPlayReason);
}

void UBMHealthBarComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bFaceCamera)
    {
        FaceCamera();
    }
}

void UBMHealthBarComponent::ObserveCharacter(ABMCharacterBase* InCharacter)
{
    if (ObservedCharacter.Get() == InCharacter)
    {
        return;
    }

    UnbindFromCharacter();
    BindToCharacter(InCharacter);
}

void UBMHealthBarComponent::SetVerticalOffset(float NewOffset)
{
    VerticalOffset = NewOffset;
    ApplyVerticalOffset();
}

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

void UBMHealthBarComponent::HandleCharacterDamaged(ABMCharacterBase* Victim, const FBMDamageInfo& Info)
{
    if (Victim == ObservedCharacter.Get())
    {
        RefreshFromStats();
    }
}

void UBMHealthBarComponent::HandleCharacterDied(ABMCharacterBase* Victim, const FBMDamageInfo& Info)
{
    if (Victim == ObservedCharacter.Get())
    {
        RefreshFromStats();
        SetVisibility(false, true);
    }
}

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

void UBMHealthBarComponent::ApplyVerticalOffset()
{
    SetRelativeLocation(FVector(0.f, 0.f, VerticalOffset));
}

UBMEnemyHealthBarComponent::UBMEnemyHealthBarComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bHideWhenFull = false;
    bFaceCamera = true;


}

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

void UBMEnemyHealthBarComponent::UpdateHealthDisplay(float CurrentHP, float MaxHP)
{
    const float Percent = (MaxHP > 0.f) ? (CurrentHP / MaxHP) : 0.f;
    const int32 FilledSegments = FMath::Clamp(FMath::RoundToInt(Percent * SegmentCount), 0, SegmentCount);


    if (ForegroundTextComponent)
    {
        ForegroundTextComponent->SetText(FText::FromString(BuildFilledBarString(FilledSegments)));
    }


}

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

