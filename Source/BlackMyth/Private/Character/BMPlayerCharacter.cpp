#include "Character/BMPlayerCharacter.h"

#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/States/BMPlayerState_Idle.h"
#include "Character/States/BMPlayerState_Move.h"
#include "Character/States/BMPlayerState_Jump.h"
#include "Character/States/BMPlayerState_Attack.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Core/BMTypes.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


ABMPlayerCharacter::ABMPlayerCharacter()
{
    CharacterType = EBMCharacterType::Player;
    Team = EBMTeam::Player;

    // === 相机臂（跟随旋转来自 Controller） ===
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 350.f;
    CameraBoom->bUsePawnControlRotation = true;   
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.f;

    // === 跟随相机 ===
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // 相机本身不转，用 Boom 转

    // 角色本体不跟着 Controller 直接转
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // 角色面向随移动方向
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = true;
        Move->RotationRate = FRotator(0.f, 720.f, 0.f);
        Move->JumpZVelocity = 600.f;
        Move->AirControl = 0.25f;
    }

    // === 绑定模型/动画资产 ===
    {
        // 1) SkeletalMesh
        static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
            TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Meshes/Wukong.Wukong'")
        );
        if (MeshFinder.Succeeded())
        {
            CharacterMeshAsset = MeshFinder.Object;
            GetMesh()->SetSkeletalMesh(CharacterMeshAsset);
        }

        // 2) 动画（AnimSequence）
        static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Idle_Zero_Pose.Idle_Zero_Pose'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> MoveFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Jog_Fwd.Jog_Fwd'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> AttackFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_A_Slow_MSA.Primary_Melee_A_Slow_MSA'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> JumpStartFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Flip_Fwd.Q_Flip_Fwd'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> FallLoopFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Fall_Loop.Q_Fall_Loop'")
        );

        if (IdleFinder.Succeeded())   AnimIdle = IdleFinder.Object;
        if (MoveFinder.Succeeded())   AnimMove = MoveFinder.Object;
        if (AttackFinder.Succeeded()) AnimLightAttack = AttackFinder.Object;
        if (JumpStartFinder.Succeeded()) AnimJumpStart = JumpStartFinder.Object;
        if (FallLoopFinder.Succeeded())  AnimFallLoop = FallLoopFinder.Object;
    }

    // === Mesh 位置/朝向 ===
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // 纯C++动画模式：单节点
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
}

void ABMPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitFSMStates();

    // 绑定 Combat 事件：输入攻击只发请求，FSM 切 Attack 由这里统一处理
    if (UBMCombatComponent* C = GetCombat())
    {
        C->OnLightAttackRequested.AddUObject(this, &ABMPlayerCharacter::OnLightAttackRequested);
    }

    // 初始动画
    PlayIdleLoop();
}

void ABMPlayerCharacter::InitFSMStates()
{
    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;

    // 创建并注册状态（Outer 用 FSM，确保生命周期跟随组件）
    UBMPlayerState_Idle* SIdle = NewObject<UBMPlayerState_Idle>(Machine);
    UBMPlayerState_Move* SMove = NewObject<UBMPlayerState_Move>(Machine);
    UBMPlayerState_Jump* SJump = NewObject<UBMPlayerState_Jump>(Machine);
    UBMPlayerState_Attack* SAtk = NewObject<UBMPlayerState_Attack>(Machine);

    SIdle->Init(this);
    SMove->Init(this);
    SJump->Init(this);
    SAtk->Init(this);

    Machine->RegisterState(BMStateNames::Idle, SIdle);
    Machine->RegisterState(BMStateNames::Move, SMove);
    Machine->RegisterState(BMStateNames::Jump, SJump);
    Machine->RegisterState(BMStateNames::Attack, SAtk);

    // 初始状态
    Machine->ChangeStateByName(BMStateNames::Idle);
}

void ABMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 传统输入映射
    // Axis: MoveForward / MoveRight
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABMPlayerCharacter::Input_MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABMPlayerCharacter::Input_MoveRight);

    // Action: Jump / Attack
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABMPlayerCharacter::Input_JumpPressed);
    PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &ABMPlayerCharacter::Input_AttackPressed);

    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ABMPlayerCharacter::Input_Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ABMPlayerCharacter::Input_LookUp);
}

void ABMPlayerCharacter::Input_MoveForward(float Value)
{
    MoveIntent.X = Value;
    UpdateMoveIntent();
}

void ABMPlayerCharacter::Input_MoveRight(float Value)
{
    MoveIntent.Y = Value;
    UpdateMoveIntent();
}

void ABMPlayerCharacter::UpdateMoveIntent()
{
    // 如果当前动作被锁（例如 Attack 状态），就不再驱动移动输入
    if (UBMCombatComponent* C = GetCombat())
    {
        if (!C->CanPerformAction())
        {
            return;
        }
    }

    if (Controller && HasMoveIntent())
    {
        const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
        const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

        AddMovementInput(Forward, MoveIntent.X);
        AddMovementInput(Right, MoveIntent.Y);
    }
}

void ABMPlayerCharacter::Input_JumpPressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        if (!C->CanPerformAction()) return;
    }
    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (!Move || Move->IsFalling())
    {
        return; // 空中不允许再次起跳
    }

    bPendingJump = true;
    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        Machine->ChangeStateByName(BMStateNames::Jump);
    }
}

void ABMPlayerCharacter::Input_AttackPressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestLightAttack(); // 触发 OnLightAttackRequested -> 切 Attack
    }
}

void ABMPlayerCharacter::Input_Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ABMPlayerCharacter::Input_LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ABMPlayerCharacter::OnLightAttackRequested()
{
    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        Machine->ChangeStateByName(BMStateNames::Attack);
    }
}

void ABMPlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    bPendingJump = false;
    // 落地后回到 Move/Idle
    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        Machine->ChangeStateByName(HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
    }
}

// ===== 纯C++动画播放 =====

void ABMPlayerCharacter::PlayLoop(UAnimSequence* Seq)
{
    if (!Seq || !GetMesh()) return;

    if (CurrentLoopAnim == Seq)
    {
        return;
    }

    CurrentLoopAnim = Seq;

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->PlayAnimation(Seq, true);
}

float ABMPlayerCharacter::PlayOnce(UAnimSequence* Seq, float PlayRate)
{
    if (!Seq || !GetMesh()) return 0.f;

    CurrentLoopAnim = nullptr;

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->PlayAnimation(Seq, false);

    const float Len = Seq->GetPlayLength();
    return (PlayRate > 0.f) ? (Len / PlayRate) : Len;
}

void ABMPlayerCharacter::PlayIdleLoop()
{
    PlayLoop(AnimIdle);
}

void ABMPlayerCharacter::PlayMoveLoop()
{
    PlayLoop(AnimMove);
}

float ABMPlayerCharacter::PlayLightAttackOnce(float PlayRate)
{
    return PlayOnce(AnimLightAttack, PlayRate);
}

float ABMPlayerCharacter::PlayJumpStartOnce(float PlayRate)
{
    return PlayOnce(AnimJumpStart, PlayRate);
}

void ABMPlayerCharacter::PlayFallLoop()
{
    PlayLoop(AnimFallLoop);
}
