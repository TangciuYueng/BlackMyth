#pragma once

#include "CoreMinimal.h"
#include "Character/BMCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BMPlayerCharacter.generated.h"


class UInputComponent;
class UAnimMontage;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class BLACKMYTH_API ABMPlayerCharacter : public ABMCharacterBase
{
    GENERATED_BODY()

public:
    ABMPlayerCharacter();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // 给状态使用：是否有移动意图
    bool HasMoveIntent() const { return MoveIntent.SizeSquared() > KINDA_SMALL_NUMBER; }

    // Jump 状态用：判断这次进入空中是不是“玩家主动起跳”
    bool ConsumePendingJump()
    {
        const bool b = bPendingJump;
        bPendingJump = false;
        return b;
    }

    // === 纯C++动画播放接口（给状态类调用）===
    void PlayIdleLoop();
    void PlayMoveLoop();
    void PlayJumpLoop();

    // === 跳跃更新 ===
    float PlayJumpStartOnce(float PlayRate = 1.0f);
    void  PlayFallLoop();

    // 播放一次性轻攻击，返回时长（Attack 状态用这个设 Timer）
    float PlayLightAttackOnce(float PlayRate = 1.0f);

    // 供 Attack 状态取 montage
    UAnimMontage* GetLightAttackMontage() const { return LightAttackMontage; }

protected:
    virtual void Landed(const FHitResult& Hit) override;

private:
    // 输入回调
    void Input_MoveForward(float Value);
    void Input_MoveRight(float Value);
    void Input_JumpPressed();
    void Input_AttackPressed();
    void Input_Turn(float Value);
    void Input_LookUp(float Value);

    // Combat 事件：由 Input_AttackPressed -> Combat->RequestLightAttack() 触发
    void OnLightAttackRequested();
    void InitFSMStates();
    void UpdateMoveIntent();

    // 内部工具：循环/一次性动画
    void PlayLoop(UAnimSequence* Seq);
    float PlayOnce(UAnimSequence* Seq, float PlayRate);

    UPROPERTY(VisibleAnywhere, Category = "BM|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "BM|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    // === 模型/动画资产===
    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<USkeletalMesh> CharacterMeshAsset;

    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimIdle;

    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimMove;

    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimJumpLoop;

    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimLightAttack;

    // 跳跃
    bool bPendingJump = false;

    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimJumpStart = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimFallLoop = nullptr;

private:
    UPROPERTY(EditAnywhere, Category = "BM|Anim")
    TObjectPtr<UAnimMontage> LightAttackMontage = nullptr;

    // 移动意图（-1..1）
    FVector2D MoveIntent = FVector2D::ZeroVector;

    // 防止重复 SetAnimation
    UPROPERTY(Transient)
    TObjectPtr<UAnimSequence> CurrentLoopAnim = nullptr;
};

