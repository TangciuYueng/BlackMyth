#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/BMTypes.h"
#include "BMCameraShakeSubsystem.generated.h"

class APlayerCameraManager;
class UCameraShakeBase;

/**
 * 相机震动参数配置
 */
USTRUCT(BlueprintType)
struct FBMCameraShakeParams
{
    GENERATED_BODY()

    // 震动持续时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake", meta = (ClampMin = "0.01"))
    float Duration = 0.15f;

    // 震动幅度（位置偏移）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake", meta = (ClampMin = "0.0"))
    float Amplitude = 5.0f;

    // 震动频率
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake", meta = (ClampMin = "0.1"))
    float Frequency = 30.0f;

    // 旋转震动幅度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake", meta = (ClampMin = "0.0"))
    float RotationAmplitude = 0.5f;

    // 震动衰减曲线指数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraShake", meta = (ClampMin = "0.1"))
    float FalloffExponent = 1.5f;

    FBMCameraShakeParams() = default;

    FBMCameraShakeParams(float InDuration, float InAmplitude, float InFrequency = 30.f, float InRotAmp = 0.5f)
        : Duration(InDuration)
        , Amplitude(InAmplitude)
        , Frequency(InFrequency)
        , RotationAmplitude(InRotAmp)
    {
    }
};

UCLASS()
class BLACKMYTH_API UBMCameraShakeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /**
     * 播放玩家受击震动（幅度较大）
     * @param Info 伤害信息，用于根据伤害类型调整震动强度
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayPlayerHitShake(const FBMDamageInfo& Info);

    /**
     * 播放敌人受击震动（幅度较小）
     * @param Info 伤害信息
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayEnemyHitShake(const FBMDamageInfo& Info);

    /**
     * 播放 Boss 受击震动
     * @param Info 伤害信息
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayBossHitShake(const FBMDamageInfo& Info);

    /**
     * 播放自定义震动
     * @param Params 震动参数
     * @param Scale 额外缩放系数
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayCustomShake(const FBMCameraShakeParams& Params, float Scale = 1.0f);

    /**
     * 停止当前震动
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void StopCurrentShake();

    // === 配置参数 ===

    // 玩家受击震动参数（轻击）
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake|Player")
    FBMCameraShakeParams PlayerHitLight = FBMCameraShakeParams(0.15f, 8.0f, 35.f, 1.0f);

    // 玩家受击震动参数（重击）
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake|Player")
    FBMCameraShakeParams PlayerHitHeavy = FBMCameraShakeParams(0.25f, 15.0f, 40.f, 2.0f);

    // 敌人受击震动参数（轻击）
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake|Enemy")
    FBMCameraShakeParams EnemyHitLight = FBMCameraShakeParams(0.08f, 2.0f, 25.f, 0.3f);

    // 敌人受击震动参数（重击）
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake|Enemy")
    FBMCameraShakeParams EnemyHitHeavy = FBMCameraShakeParams(0.12f, 4.0f, 30.f, 0.6f);

    // Boss 受击震动参数（轻击）
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake|Boss")
    FBMCameraShakeParams BossHitLight = FBMCameraShakeParams(0.10f, 3.0f, 28.f, 0.5f);

    // Boss 受击震动参数（重击）
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake|Boss")
    FBMCameraShakeParams BossHitHeavy = FBMCameraShakeParams(0.18f, 6.0f, 35.f, 1.0f);

    // 全局震动强度缩放
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float GlobalShakeScale = 1.0f;

    // 是否启用相机震动
    UPROPERTY(EditAnywhere, Category = "BM|CameraShake")
    bool bEnableCameraShake = true;

private:
    /**
     * 内部执行震动
     */
    void ExecuteShake(const FBMCameraShakeParams& Params, float Scale);

    /**
     * 获取玩家相机管理器
     */
    APlayerCameraManager* GetPlayerCameraManager() const;

    /**
     * 应用相机偏移
     */
    void ApplyCameraOffset(const FVector& Offset, const FRotator& Rotation);

    /**
     * 重置相机偏移到初始状态
     */
    void ResetCameraOffset();

    // 当前震动状态
    FTimerHandle ShakeTimerHandle;
    float CurrentShakeTime = 0.f;
    float CurrentShakeDuration = 0.f;
    FBMCameraShakeParams CurrentShakeParams;
    float CurrentShakeScale = 1.0f;

    // 震动偏移累积
    FVector AccumulatedOffset = FVector::ZeroVector;
    FRotator AccumulatedRotation = FRotator::ZeroRotator;

    /**
     * 每帧更新震动
     */
    void TickShake();
};
