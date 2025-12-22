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

    TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("EnemyHealthText"));
    if (TextComponent)
    {
        TextComponent->SetupAttachment(this);
        TextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
        TextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
        TextComponent->SetTextRenderColor(FColor::White);
        TextComponent->SetWorldSize(FontWorldSize);
        TextComponent->SetText(FText::FromString(TEXT("[----------] 0%")));
        TextComponent->SetHiddenInGame(false);
        TextComponent->SetTranslucentSortPriority(5);
    }
}

void UBMEnemyHealthBarComponent::OnRegister()
{
    Super::OnRegister();

    if (TextComponent)
    {
        TextComponent->SetWorldSize(FontWorldSize);
    }
}

void UBMEnemyHealthBarComponent::UpdateHealthDisplay(float CurrentHP, float MaxHP)
{
    if (!TextComponent)
    {
        return;
    }

    TextComponent->SetText(FText::FromString(BuildBarString(CurrentHP, MaxHP)));
}

FString UBMEnemyHealthBarComponent::BuildBarString(float CurrentHP, float MaxHP) const
{
    const float Percent = (MaxHP > 0.f) ? (CurrentHP / MaxHP) : 0.f;
    const int32 FilledSegments = FMath::Clamp(FMath::RoundToInt(Percent * SegmentCount), 0, SegmentCount);

    FString Bar("[");
    for (int32 Index = 0; Index < SegmentCount; ++Index)
    {
        Bar.AppendChar(Index < FilledSegments ? '#' : '-');
    }
    Bar.AppendChar(']');
    Bar.AppendChar(' ');
    Bar.AppendInt(FMath::RoundToInt(Percent * 100.f));
    Bar.AppendChar('%');

    return Bar;
}

