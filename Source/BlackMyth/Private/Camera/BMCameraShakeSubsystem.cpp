#include "Camera/BMCameraShakeSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBMCameraShake, Log, All);

/*
 * @brief Initialize the camera shake subsystem
 * @param Collection The subsystem collection
 */
void UBMCameraShakeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogBMCameraShake, Log, TEXT("BMCameraShakeSubsystem initialized"));
}

/*
 * @brief Deinitialize the camera shake subsystem
 */
void UBMCameraShakeSubsystem::Deinitialize()
{
    StopCurrentShake();
    Super::Deinitialize();
    UE_LOG(LogBMCameraShake, Log, TEXT("BMCameraShakeSubsystem deinitialized"));
}

/*
 * @brief Should create the camera shake subsystem
 * @param Outer The outer object
 * @return True if the subsystem should be created, false otherwise
 */
bool UBMCameraShakeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (const UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

/*
 * @brief Play the player hit shake
 * @param Info The damage info
 */
void UBMCameraShakeSubsystem::PlayPlayerHitShake(const FBMDamageInfo& Info)
{
    if (!bEnableCameraShake) return;

    const bool bHeavy = BMCombatUtils::IsHeavyIncoming(Info);
    const FBMCameraShakeParams& Params = bHeavy ? PlayerHitHeavy : PlayerHitLight;

    float DamageScale = 1.0f;
    if (Info.DamageValue > 100.f)
    {
        DamageScale = FMath::Clamp(Info.DamageValue / 100.f, 1.0f, 2.0f);
    }

    UE_LOG(LogBMCameraShake, Verbose, TEXT("PlayPlayerHitShake: Heavy=%d, Damage=%.1f, Scale=%.2f"),
        bHeavy, Info.DamageValue, DamageScale);

    ExecuteShake(Params, DamageScale);
}

/*
 * @brief Play the enemy hit shake
 * @param Info The damage info
 */
void UBMCameraShakeSubsystem::PlayEnemyHitShake(const FBMDamageInfo& Info)
{
    if (!bEnableCameraShake) return;

    const bool bHeavy = BMCombatUtils::IsHeavyIncoming(Info);
    const FBMCameraShakeParams& Params = bHeavy ? EnemyHitHeavy : EnemyHitLight;

    UE_LOG(LogBMCameraShake, Verbose, TEXT("PlayEnemyHitShake: Heavy=%d, Damage=%.1f"),
        bHeavy, Info.DamageValue);

    ExecuteShake(Params, 1.0f);
}

/*
 * @brief Play the boss hit shake
 * @param Info The damage info
 */
void UBMCameraShakeSubsystem::PlayBossHitShake(const FBMDamageInfo& Info)
{
    if (!bEnableCameraShake) return;

    const bool bHeavy = BMCombatUtils::IsHeavyIncoming(Info);
    const FBMCameraShakeParams& Params = bHeavy ? BossHitHeavy : BossHitLight;

    UE_LOG(LogBMCameraShake, Verbose, TEXT("PlayBossHitShake: Heavy=%d, Damage=%.1f"),
        bHeavy, Info.DamageValue);

    ExecuteShake(Params, 1.0f);
}

/*
 * @brief Play the custom shake
 * @param Params The shake params
 * @param Scale The scale
 */
void UBMCameraShakeSubsystem::PlayCustomShake(const FBMCameraShakeParams& Params, float Scale)
{
    if (!bEnableCameraShake) return;

    ExecuteShake(Params, Scale);
}

/*
 * @brief Stop the current shake
 */
void UBMCameraShakeSubsystem::StopCurrentShake()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ShakeTimerHandle);
    }

    ResetCameraOffset();

    CurrentShakeTime = 0.f;
    CurrentShakeDuration = 0.f;
    AccumulatedOffset = FVector::ZeroVector;
    AccumulatedRotation = FRotator::ZeroRotator;
}

/*
 * @brief Execute the shake
 * @param Params The shake params
 * @param Scale The scale
 */
void UBMCameraShakeSubsystem::ExecuteShake(const FBMCameraShakeParams& Params, float Scale)
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (ShakeTimerHandle.IsValid())
    {
        const float NewIntensity = Params.Amplitude * Scale;
        const float CurrentIntensity = CurrentShakeParams.Amplitude * CurrentShakeScale;
        
        // �������ǿ�ȸ��󣬻��ߵ�ǰ���Ѿ�˥������һ�룬�򸲸�
        const float NormalizedTime = CurrentShakeDuration > 0.f ? CurrentShakeTime / CurrentShakeDuration : 1.f;
        if (NewIntensity < CurrentIntensity && NormalizedTime < 0.5f)
        {
            // ���Խ�������
            return;
        }
    }

    // ֹ֮ͣǰ����
    StopCurrentShake();

    // �����µ��𶯲���
    CurrentShakeParams = Params;
    CurrentShakeScale = Scale * GlobalShakeScale;
    CurrentShakeTime = 0.f;
    CurrentShakeDuration = Params.Duration;

    // ������ʱ����ÿ֡������
    const float TickInterval = 1.f / 60.f; 
    World->GetTimerManager().SetTimer(
        ShakeTimerHandle,
        this,
        &UBMCameraShakeSubsystem::TickShake,
        TickInterval,
        true
    );
}

/*
 * @brief Tick the shake
 */
void UBMCameraShakeSubsystem::TickShake()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // ����ʱ��
    const float DeltaTime = World->GetDeltaSeconds();
    CurrentShakeTime += DeltaTime;

    // ����Ƿ����
    if (CurrentShakeTime >= CurrentShakeDuration)
    {
        StopCurrentShake();
        return;
    }

    // ����˥������
    const float NormalizedTime = CurrentShakeTime / CurrentShakeDuration;
    const float Falloff = FMath::Pow(1.0f - NormalizedTime, CurrentShakeParams.FalloffExponent);

    // ���㵱ǰ��ƫ��
    const float Time = CurrentShakeTime * CurrentShakeParams.Frequency;
    const float Amplitude = CurrentShakeParams.Amplitude * CurrentShakeScale * Falloff;
    const float RotAmplitude = CurrentShakeParams.RotationAmplitude * CurrentShakeScale * Falloff;

    // ʹ�� Perlin ����������
    FVector NewOffset;
    NewOffset.X = Amplitude * (FMath::Sin(Time * 1.0f) * 0.5f + FMath::Sin(Time * 2.3f) * 0.3f + FMath::Sin(Time * 4.1f) * 0.2f);
    NewOffset.Y = Amplitude * (FMath::Sin(Time * 1.3f + 1.5f) * 0.5f + FMath::Sin(Time * 2.7f) * 0.3f + FMath::Sin(Time * 3.9f) * 0.2f);
    NewOffset.Z = Amplitude * (FMath::Sin(Time * 0.9f + 3.0f) * 0.5f + FMath::Sin(Time * 2.1f) * 0.3f + FMath::Sin(Time * 4.3f) * 0.2f);

    FRotator NewRotation;
    NewRotation.Pitch = RotAmplitude * FMath::Sin(Time * 1.1f + 0.5f);
    NewRotation.Yaw = RotAmplitude * FMath::Sin(Time * 1.4f + 1.0f) * 0.5f;
    NewRotation.Roll = RotAmplitude * FMath::Sin(Time * 0.8f + 2.0f) * 0.3f;

    // Ӧ����ƫ��
    ApplyCameraOffset(NewOffset, NewRotation);

    // ��¼��ǰƫ��
    AccumulatedOffset = NewOffset;
    AccumulatedRotation = NewRotation;
}

/*
 * @brief Apply the camera offset
 * @param Offset The offset
 * @param Rotation The rotation
 */
void UBMCameraShakeSubsystem::ApplyCameraOffset(const FVector& Offset, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    // ���Ի�ȡ CameraComponent
    UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();
    if (Camera)
    {
        // �������ƫ������
        const FVector DeltaOffset = Offset - AccumulatedOffset;
        const FRotator DeltaRotation = Rotation - AccumulatedRotation;

        // Ӧ��ƫ�Ƶ����
        FVector CurrentRelative = Camera->GetRelativeLocation();
        FRotator CurrentRelativeRot = Camera->GetRelativeRotation();

        Camera->SetRelativeLocation(CurrentRelative + DeltaOffset);
        Camera->SetRelativeRotation(CurrentRelativeRot + DeltaRotation);
    }
}

/*
 * @brief Reset the camera offset
 */
void UBMCameraShakeSubsystem::ResetCameraOffset()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();
    if (Camera && (!AccumulatedOffset.IsNearlyZero() || !AccumulatedRotation.IsNearlyZero()))
    {
        // �����ۻ���ƫ��
        FVector CurrentRelative = Camera->GetRelativeLocation();
        FRotator CurrentRelativeRot = Camera->GetRelativeRotation();

        Camera->SetRelativeLocation(CurrentRelative - AccumulatedOffset);
        Camera->SetRelativeRotation(CurrentRelativeRot - AccumulatedRotation);
    }
}

/*
 * @brief Get the player camera manager
 * @return The player camera manager
 */
APlayerCameraManager* UBMCameraShakeSubsystem::GetPlayerCameraManager() const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->PlayerCameraManager;
        }
    }
    return nullptr;
}
