#include "Camera/BMCameraShakeSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBMCameraShake, Log, All);

void UBMCameraShakeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogBMCameraShake, Log, TEXT("BMCameraShakeSubsystem initialized"));
}

void UBMCameraShakeSubsystem::Deinitialize()
{
    StopCurrentShake();
    Super::Deinitialize();
    UE_LOG(LogBMCameraShake, Log, TEXT("BMCameraShakeSubsystem deinitialized"));
}

bool UBMCameraShakeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (const UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

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

void UBMCameraShakeSubsystem::PlayEnemyHitShake(const FBMDamageInfo& Info)
{
    if (!bEnableCameraShake) return;

    const bool bHeavy = BMCombatUtils::IsHeavyIncoming(Info);
    const FBMCameraShakeParams& Params = bHeavy ? EnemyHitHeavy : EnemyHitLight;

    UE_LOG(LogBMCameraShake, Verbose, TEXT("PlayEnemyHitShake: Heavy=%d, Damage=%.1f"),
        bHeavy, Info.DamageValue);

    ExecuteShake(Params, 1.0f);
}

void UBMCameraShakeSubsystem::PlayBossHitShake(const FBMDamageInfo& Info)
{
    if (!bEnableCameraShake) return;

    const bool bHeavy = BMCombatUtils::IsHeavyIncoming(Info);
    const FBMCameraShakeParams& Params = bHeavy ? BossHitHeavy : BossHitLight;

    UE_LOG(LogBMCameraShake, Verbose, TEXT("PlayBossHitShake: Heavy=%d, Damage=%.1f"),
        bHeavy, Info.DamageValue);

    ExecuteShake(Params, 1.0f);
}

void UBMCameraShakeSubsystem::PlayCustomShake(const FBMCameraShakeParams& Params, float Scale)
{
    if (!bEnableCameraShake) return;

    ExecuteShake(Params, Scale);
}

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

void UBMCameraShakeSubsystem::ExecuteShake(const FBMCameraShakeParams& Params, float Scale)
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (ShakeTimerHandle.IsValid())
    {
        const float NewIntensity = Params.Amplitude * Scale;
        const float CurrentIntensity = CurrentShakeParams.Amplitude * CurrentShakeScale;
        
        // 如果新震动强度更大，或者当前震动已经衰减超过一半，则覆盖
        const float NormalizedTime = CurrentShakeDuration > 0.f ? CurrentShakeTime / CurrentShakeDuration : 1.f;
        if (NewIntensity < CurrentIntensity && NormalizedTime < 0.5f)
        {
            // 忽略较弱的震动
            return;
        }
    }

    // 停止之前的震动
    StopCurrentShake();

    // 设置新的震动参数
    CurrentShakeParams = Params;
    CurrentShakeScale = Scale * GlobalShakeScale;
    CurrentShakeTime = 0.f;
    CurrentShakeDuration = Params.Duration;

    // 启动定时器，每帧更新震动
    const float TickInterval = 1.f / 60.f; // 60fps 更新
    World->GetTimerManager().SetTimer(
        ShakeTimerHandle,
        this,
        &UBMCameraShakeSubsystem::TickShake,
        TickInterval,
        true
    );
}

void UBMCameraShakeSubsystem::TickShake()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 更新时间
    const float DeltaTime = World->GetDeltaSeconds();
    CurrentShakeTime += DeltaTime;

    // 检查是否结束
    if (CurrentShakeTime >= CurrentShakeDuration)
    {
        StopCurrentShake();
        return;
    }

    // 计算衰减因子（从1到0）
    const float NormalizedTime = CurrentShakeTime / CurrentShakeDuration;
    const float Falloff = FMath::Pow(1.0f - NormalizedTime, CurrentShakeParams.FalloffExponent);

    // 计算当前震动偏移
    const float Time = CurrentShakeTime * CurrentShakeParams.Frequency;
    const float Amplitude = CurrentShakeParams.Amplitude * CurrentShakeScale * Falloff;
    const float RotAmplitude = CurrentShakeParams.RotationAmplitude * CurrentShakeScale * Falloff;

    // 使用 Perlin 噪声风格的震动（多个正弦波叠加）
    FVector NewOffset;
    NewOffset.X = Amplitude * (FMath::Sin(Time * 1.0f) * 0.5f + FMath::Sin(Time * 2.3f) * 0.3f + FMath::Sin(Time * 4.1f) * 0.2f);
    NewOffset.Y = Amplitude * (FMath::Sin(Time * 1.3f + 1.5f) * 0.5f + FMath::Sin(Time * 2.7f) * 0.3f + FMath::Sin(Time * 3.9f) * 0.2f);
    NewOffset.Z = Amplitude * (FMath::Sin(Time * 0.9f + 3.0f) * 0.5f + FMath::Sin(Time * 2.1f) * 0.3f + FMath::Sin(Time * 4.3f) * 0.2f);

    FRotator NewRotation;
    NewRotation.Pitch = RotAmplitude * FMath::Sin(Time * 1.1f + 0.5f);
    NewRotation.Yaw = RotAmplitude * FMath::Sin(Time * 1.4f + 1.0f) * 0.5f;
    NewRotation.Roll = RotAmplitude * FMath::Sin(Time * 0.8f + 2.0f) * 0.3f;

    // 应用震动偏移
    ApplyCameraOffset(NewOffset, NewRotation);

    // 记录当前偏移
    AccumulatedOffset = NewOffset;
    AccumulatedRotation = NewRotation;
}

void UBMCameraShakeSubsystem::ApplyCameraOffset(const FVector& Offset, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    // 尝试获取 CameraComponent
    UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();
    if (Camera)
    {
        // 计算相对偏移增量
        const FVector DeltaOffset = Offset - AccumulatedOffset;
        const FRotator DeltaRotation = Rotation - AccumulatedRotation;

        // 应用偏移到相机
        FVector CurrentRelative = Camera->GetRelativeLocation();
        FRotator CurrentRelativeRot = Camera->GetRelativeRotation();

        Camera->SetRelativeLocation(CurrentRelative + DeltaOffset);
        Camera->SetRelativeRotation(CurrentRelativeRot + DeltaRotation);
    }
}

void UBMCameraShakeSubsystem::ResetCameraOffset()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    UCameraComponent* Camera = PlayerPawn->FindComponentByClass<UCameraComponent>();
    if (Camera && (!AccumulatedOffset.IsNearlyZero() || !AccumulatedRotation.IsNearlyZero()))
    {
        // 撤销累积的偏移
        FVector CurrentRelative = Camera->GetRelativeLocation();
        FRotator CurrentRelativeRot = Camera->GetRelativeRotation();

        Camera->SetRelativeLocation(CurrentRelative - AccumulatedOffset);
        Camera->SetRelativeRotation(CurrentRelativeRot - AccumulatedRotation);
    }
}

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
