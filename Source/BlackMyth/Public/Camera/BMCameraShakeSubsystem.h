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

    /**
     * 默认构造函数
     */
    FBMCameraShakeParams() = default;

    /**
     * 参数化构造函数
     *
     * @param InDuration 震动持续时间（秒），控制震动效果的总时长
     * @param InAmplitude 震动幅度（位置偏移），控制相机位置偏移的强度
     * @param InFrequency 震动频率，控制震动的快慢，值越大震动越快，默认为 30.0
     * @param InRotAmp 旋转震动幅度，控制相机旋转震动的强度，默认为 0.5
     */
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
    /**
     * 初始化子系统
     *
     * 在子系统创建时调用，用于初始化相机震动相关的资源和状态
     *
     * @param Collection 子系统集合，包含所有已注册的子系统
     */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /**
     * 反初始化子系统
     *
     * 在子系统销毁时调用，清理所有震动状态并停止当前正在进行的震动
     */
    virtual void Deinitialize() override;

    /**
     * 判断是否应该创建该子系统
     *
     * 仅在游戏世界中创建此子系统
     *
     * @param Outer 外部对象，通常是 UWorld 实例
     * @return 若应该创建子系统则返回 true，否则返回 false
     */
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    /**
     * 播放玩家受击震动
     *
     * 当玩家角色受到伤害时触发相机震动效果，震动幅度较大以强调玩家受击感
     * 根据伤害类型（轻击/重击）和伤害值自动选择合适的震动参数和强度缩放
     *
     * @param Info 伤害信息，包含伤害值、伤害类型、受击反应等级等数据，用于判断震动强度
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayPlayerHitShake(const FBMDamageInfo& Info);

    /**
     * 播放敌人受击震动
     *
     * 当敌人角色受到玩家攻击时触发相机震动效果，震动幅度较小以提供攻击反馈
     * 根据伤害类型（轻击/重击）自动选择合适的震动参数
     *
     * @param Info 伤害信息，包含伤害值、伤害类型、受击反应等级等数据，用于判断震动类型
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayEnemyHitShake(const FBMDamageInfo& Info);

    /**
     * 播放 Boss 受击震动
     *
     * 当 Boss 角色受到玩家攻击时触发相机震动效果，震动幅度介于玩家和普通敌人之间
     * 根据伤害类型（轻击/重击）自动选择合适的震动参数
     *
     * @param Info 伤害信息，包含伤害值、伤害类型、受击反应等级等数据，用于判断震动类型
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayBossHitShake(const FBMDamageInfo& Info);

    /**
     * 播放自定义震动
     *
     * 使用自定义参数触发相机震动效果
     *
     * @param Params 震动参数配置，包含持续时间、位置幅度、旋转幅度、频率、衰减指数等
     * @param Scale 额外的震动强度缩放系数，默认为 1.0；会与全局震动缩放系数叠加
     */
    UFUNCTION(BlueprintCallable, Category = "BM|CameraShake")
    void PlayCustomShake(const FBMCameraShakeParams& Params, float Scale = 1.0f);

    /**
     * 停止当前震动
     *
     * 立即停止正在进行的相机震动效果，清除震动定时器并将相机位置和旋转恢复到震动前的状态
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
     *
     * 所有公共震动函数的统一执行入口，负责停止旧震动、设置新震动参数、启动震动定时器
     * 如果新震动强度较弱且当前震动尚未衰减过半，则会忽略新震动请求
     *
     * @param Params 震动参数配置，包含持续时间、幅度、频率等
     * @param Scale 震动强度缩放系数，会与全局震动缩放系数叠加后应用
     */
    void ExecuteShake(const FBMCameraShakeParams& Params, float Scale);

    /**
     * 获取玩家相机管理器
     *
     * 从当前世界的第一个玩家控制器中获取相机管理器实例
     *
     * @return 玩家相机管理器指针，若未找到则返回 nullptr
     */
    APlayerCameraManager* GetPlayerCameraManager() const;

    /**
     * 应用相机偏移
     *
     * 将震动产生的位置和旋转偏移增量应用到玩家相机组件上
     *
     * @param Offset 目标位置偏移向量（相对于相机初始位置）
     * @param Rotation 目标旋转偏移（相对于相机初始旋转）
     */
    void ApplyCameraOffset(const FVector& Offset, const FRotator& Rotation);

    /**
     * 重置相机偏移到初始状态
     *
     * 将相机的位置和旋转恢复到震动开始前的状态，通过减去累积的偏移量实现
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
     *
     * 定时器回调函数，每帧计算当前震动的偏移量并应用到相机
     * 使用基于时间的衰减和多频率正弦波叠加生成平滑的震动效果
     * 当震动时间超过持续时间时自动停止震动
     */
    void TickShake();
};
