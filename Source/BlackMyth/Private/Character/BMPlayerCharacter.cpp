#include "Character/BMPlayerCharacter.h"

#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/States/BMPlayerState_Idle.h"
#include "Character/States/BMPlayerState_Move.h"
#include "Character/States/BMPlayerState_Jump.h"
#include "Character/States/BMPlayerState_Attack.h"
#include "Character/States/BMPlayerState_Hit.h"
#include "Character/States/BMPlayerState_Death.h"
#include "Character/States/BMPlayerState_Dodge.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Core/BMTypes.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Character/Components/BMHurtBoxComponent.h"
#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMStatsComponent.h"

#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimSingleNodeInstance.h"


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
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeChannel = ECC_GameTraceChannel1; // 只让墙/地形影响相机
    CameraBoom->ProbeSize = 12.f;

    CameraBoom->bEnableCameraLag = true;
    CameraBoom->bUseCameraLagSubstepping = true;
    CameraBoom->CameraLagMaxTimeStep = 1.f / 60.f;
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
        static ConstructorHelpers::FObjectFinder<UAnimSequence> LightAttackFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_A_Slow_MSA.Primary_Melee_A_Slow_MSA'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> HeavyAttackFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/RMB_Hit.RMB_Hit'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> JumpStartFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Flip_Fwd.Q_Flip_Fwd'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> FallLoopFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Fall_Loop.Q_Fall_Loop'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> HitLightFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/HitReact_Front.HitReact_Front'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> HitHeavyFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/HitReact_Front.HitReact_Front'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> DeathFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Death.Death'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> DodgeFinder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Q_Flip_Fwd.Q_Flip_Fwd'")
        );
        if (IdleFinder.Succeeded())            AnimIdle = IdleFinder.Object;
        if (MoveFinder.Succeeded())            AnimMove = MoveFinder.Object;
        if (LightAttackFinder.Succeeded())     AnimLightAttack = LightAttackFinder.Object;
        if (HeavyAttackFinder.Succeeded())     AnimHeavyAttack = HeavyAttackFinder.Object;
        if (JumpStartFinder.Succeeded())       AnimJumpStart = JumpStartFinder.Object;
        if (FallLoopFinder.Succeeded())        AnimFallLoop = FallLoopFinder.Object;
        if (HitLightFinder.Succeeded())        AnimHitLight = HitLightFinder.Object;
        if (HitHeavyFinder.Succeeded())        AnimHitHeavy = HitHeavyFinder.Object;
        if (DeathFinder.Succeeded())           AnimDeath = DeathFinder.Object;
        if (DodgeFinder.Succeeded())           AnimDodge = DodgeFinder.Object;
    }

    // === Mesh 位置/朝向 ===
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // 纯C++动画模式：单节点
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    // === HurtBoxes ===
    UBMHurtBoxComponent* Head = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Head"));
    Head->AttachSocketOrBone = TEXT("head");
    Head->BoxExtent = FVector(16, 16, 16);
    Head->DamageMultiplier = 1.6f; // 头部更疼

    UBMHurtBoxComponent* Body = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Body"));
    Body->AttachSocketOrBone = TEXT("spine_03");
    Body->BoxExtent = FVector(20, 25, 30);
    Body->DamageMultiplier = 1.0f;

    // === HitBox 定义 ===
    if (UBMHitBoxComponent* HB = GetHitBox())
    {
        FBMHitBoxDefinition Light;
        Light.Name = TEXT("LightAttack");
        Light.Type = EBMHitBoxType::LightAttack;
        Light.AttachSocketOrBone = TEXT("weapon_r");  
        Light.BoxExtent = FVector(8, 108, 8);
        Light.DamageType = EBMDamageType::Melee;
        Light.ElementType = EBMElementType::Physical;
        Light.DamageScale = 1.0f;
        Light.AdditiveDamage = 0.f;
        Light.DefaultReaction = EBMHitReaction::Light;
        Light.KnockbackStrength = 120.f;

        HB->RegisterDefinition(Light);

        FBMHitBoxDefinition Heavy;
        Heavy.Name = TEXT("HeavyAttack");
        Heavy.Type = EBMHitBoxType::HeavyAttack;
        Heavy.AttachSocketOrBone = TEXT("weapon_r");
        Heavy.BoxExtent = FVector(8, 150, 8);
        Heavy.RelativeTransform = FTransform(
            FRotator::ZeroRotator,
            FVector(0.f, -150.f, 0.f),
            FVector::OneVector
        );

        Heavy.DamageType = EBMDamageType::Melee;
        Heavy.ElementType = EBMElementType::Physical;
        Heavy.DamageScale = 1.35f;
        Heavy.AdditiveDamage = 0.f;
        Heavy.DefaultReaction = EBMHitReaction::Heavy;
        Heavy.KnockbackStrength = 180.f;

        HB->RegisterDefinition(Heavy);
    }

    LightAttackSpecs.Reset();
    {
        FBMPlayerAttackSpec S;
        S.Id = TEXT("Light_01");
        S.Anim = AnimLightAttack;
        S.MaxPlayTime = 0.30f;

        S.HitBoxNames = { TEXT("LightAttack") };

        S.HitBoxParams.bOverrideReaction = true;
        S.HitBoxParams.OverrideReaction = EBMHitReaction::Light;

        S.InterruptChance = 0.65f;
        S.InterruptChanceOnHeavyHit = 1.0f;

        S.Cooldown = 1.0f;
        LightAttackSpecs.Add(S);
    }


    HeavyAttackSpecs.Reset();
    {
        FBMPlayerAttackSpec S;
        S.Id = TEXT("Heavy_01");
        S.Anim = AnimHeavyAttack;

        S.HitBoxNames = { TEXT("HeavyAttack") };

        S.HitBoxParams.DamageMultiplier = 1.25f;
        S.HitBoxParams.bOverrideReaction = true;
        S.HitBoxParams.OverrideReaction = EBMHitReaction::Heavy;

        S.bUninterruptible = true;
        S.InterruptChance = 0.1f;
        S.InterruptChanceOnHeavyHit = 0.3f;

        S.Cooldown = 2.0f;
        HeavyAttackSpecs.Add(S);
    }
}

void ABMPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitFSMStates();

    // 绑定 Combat 事件：输入攻击只发请求，FSM 切 Attack 由这里统一处理
    if (UBMCombatComponent* C = GetCombat())
    {
        C->OnActionRequested.AddUObject(this, &ABMPlayerCharacter::OnActionRequested);
    }

    // 初始动画
    PlayIdleLoop();

	// 调试：启用 HitBox/HurtBox 可视化
    if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    for (UBMHurtBoxComponent* HB : HurtBoxes)
    {
        if (!HB) continue;
        HB->bDebugDraw = true;
    }
}

void ABMPlayerCharacter::InitFSMStates()
{
    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;

    // 创建并注册状态（Outer 用 FSM，确保生命周期跟随组件）
    auto* SIdle = NewObject<UBMPlayerState_Idle>(Machine);
    auto* SMove = NewObject<UBMPlayerState_Move>(Machine);
    auto* SJump = NewObject<UBMPlayerState_Jump>(Machine);
    auto* SAtk = NewObject<UBMPlayerState_Attack>(Machine);
    auto* SHit = NewObject<UBMPlayerState_Hit>(Machine);
    auto* SDeath = NewObject<UBMPlayerState_Death>(Machine);
    auto* SDodge = NewObject<UBMPlayerState_Dodge>(Machine);

    SIdle->Init(this);
    SMove->Init(this);
    SJump->Init(this);
    SAtk->Init(this);
    SHit->Init(this);
    SDeath->Init(this);
    SDodge->Init(this);

    Machine->RegisterState(BMStateNames::Idle, SIdle);
    Machine->RegisterState(BMStateNames::Move, SMove);
    Machine->RegisterState(BMStateNames::Jump, SJump);
    Machine->RegisterState(BMStateNames::Attack, SAtk);
    Machine->RegisterState(BMStateNames::Hit, SHit);
    Machine->RegisterState(BMStateNames::Death, SDeath);
    Machine->RegisterState(BMStateNames::Dodge, SDodge);

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
    PlayerInputComponent->BindAction(TEXT("LightAttack"), IE_Pressed, this, &ABMPlayerCharacter::Input_AttackLightPressed);
    PlayerInputComponent->BindAction(TEXT("HeavyAttack"), IE_Pressed, this, &ABMPlayerCharacter::Input_AttackHeavyPressed);
    PlayerInputComponent->BindAction(TEXT("Dodge"), IE_Pressed, this, &ABMPlayerCharacter::Input_DodgePressed);

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

void ABMPlayerCharacter::Input_AttackLightPressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::LightAttack);
    }
}

void ABMPlayerCharacter::Input_AttackHeavyPressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::HeavyAttack);
    }
}

void ABMPlayerCharacter::Input_DodgePressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::Dodge);
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

void ABMPlayerCharacter::OnActionRequested(EBMCombatAction Action)
{
    // 死亡不响应
    if (UBMStatsComponent* S = GetStats())
    {
        if (S->IsDead()) return;
    }

    if ((Action == EBMCombatAction::LightAttack || Action == EBMCombatAction::HeavyAttack))
    {
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            if (Move->IsFalling())
            {
                return;
            }
        }
    }

    // Dodge：不允许空中
    if (Action == EBMCombatAction::Dodge)
    {
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            if (Move->IsFalling()) return;
        }

        // 冷却检查
        if (UBMCombatComponent* C = GetCombat())
        {
            if (!C->IsCooldownReady(DodgeCooldownKey))
            {
                return;
            }
        }

        if (UBMStateMachineComponent* Machine = GetFSM())
        {
            Machine->ChangeStateByName(BMStateNames::Dodge);
        }
        return;
    }


    PendingAction = Action;

    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        if (Action == EBMCombatAction::LightAttack || Action == EBMCombatAction::HeavyAttack)
        {
            Machine->ChangeStateByName(BMStateNames::Attack);
        }
        else
        {
            // 
            UE_LOG(LogTemp, Log, TEXT("Skill action requested (not implemented): %d"), (int32)Action);
        }
    }
}

bool ABMPlayerCharacter::ConsumePendingAction(EBMCombatAction& OutAction)
{
    if (PendingAction == EBMCombatAction::None) return false;
    OutAction = PendingAction;
    PendingAction = EBMCombatAction::None;
    return true;
}


static bool SelectWeighted(const TArray<FBMPlayerAttackSpec>& Specs, const UBMCombatComponent* Combat, FBMPlayerAttackSpec& Out)
{
    float TotalW = 0.f;
    TArray<const FBMPlayerAttackSpec*> Cands;

    for (const FBMPlayerAttackSpec& S : Specs)
    {
        if (Combat && !Combat->IsCooldownReady(S.Id))
        {
            continue;
        }
        if (!S.Anim) continue;
        const float W = FMath::Max(0.01f, S.Weight);
        TotalW += W;
        Cands.Add(&S);
    }
    if (Cands.Num() == 0) return false;

    float R = FMath::FRandRange(0.f, TotalW);
    for (const FBMPlayerAttackSpec* P : Cands)
    {
        R -= FMath::Max(0.01f, P->Weight);
        if (R <= 0.f)
        {
            Out = *P;
            return true;
        }
    }
    Out = *Cands.Last();
    return true;
}

bool ABMPlayerCharacter::SelectAttackSpec(EBMCombatAction Action, FBMPlayerAttackSpec& OutSpec) const
{
    
    if (Action == EBMCombatAction::LightAttack)
    {
        return SelectWeighted(LightAttackSpecs, Combat, OutSpec);
    }
    if (Action == EBMCombatAction::HeavyAttack)
    {
        return SelectWeighted(HeavyAttackSpecs, Combat, OutSpec);
    }
    return false;
}

void ABMPlayerCharacter::SetActiveAttackSpec(const FBMPlayerAttackSpec& Spec)
{
    ActiveAttackSpec = Spec;
    bHasActiveAttackSpec = true;
}

void ABMPlayerCharacter::ClearActiveAttackSpec()
{
    bHasActiveAttackSpec = false;
}

bool ABMPlayerCharacter::ShouldInterruptCurrentAttack(const FBMDamageInfo& Incoming) const
{
    if (!bHasActiveAttackSpec)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] No ActiveAttackSpec while attacking."), *GetName());
        return true;
    }

    const FBMPlayerAttackSpec& Spec = ActiveAttackSpec;

    if (Spec.bUninterruptible)
    {
        return false;
    }

    float P = BMCombatUtils::IsHeavyIncoming(Incoming) ? Spec.InterruptChanceOnHeavyHit : Spec.InterruptChance;
    P = FMath::Clamp(P, 0.f, 1.f);

    return FMath::FRand() < P;
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

float ABMPlayerCharacter::PlayOnce(UAnimSequence* Seq, float PlayRate, float StartTime, float MaxPlayTime)
{
    if (!Seq || !GetMesh())
    {
        return 0.f;
    }

    CurrentLoopAnim = nullptr;

    // 确保是单节点模式
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    // 用 SingleNodeInstance 控制起始时间/播放率
    UAnimSingleNodeInstance* Inst = GetMesh()->GetSingleNodeInstance();

    // 第一次没创建 Instance，先 PlayAnimation 一次
    if (!Inst)
    {
        GetMesh()->PlayAnimation(Seq, false);
        Inst = GetMesh()->GetSingleNodeInstance();
    }

    const float Len = Seq->GetPlayLength();
    const float SafePlayRate = (PlayRate > 0.f) ? PlayRate : 1.0f;

    const float ClampedStart = FMath::Clamp(StartTime, 0.f, Len);
    const float Remaining = FMath::Max(0.f, Len - ClampedStart);

    // MaxPlayTime <= 0 表示播完整剩余段，否则裁剪
    const float EffectivePlayTime = (MaxPlayTime > 0.f) ? FMath::Min(Remaining, MaxPlayTime) : Remaining;

    if (Inst)
    {
        Inst->SetAnimationAsset(Seq, /*bIsLooping=*/false);
        Inst->SetPosition(ClampedStart, /*bFireNotifies=*/true);
        Inst->SetPlayRate(SafePlayRate);
        Inst->SetPlaying(true);
    }
    else
    {
        // 若无法控制起始时间/裁剪
        GetMesh()->PlayAnimation(Seq, false);
        return Len / SafePlayRate;
    }

    // 返回时长
    return EffectivePlayTime / SafePlayRate;
}

void ABMPlayerCharacter::PlayIdleLoop()
{
    PlayLoop(AnimIdle);
}

void ABMPlayerCharacter::PlayMoveLoop()
{
    PlayLoop(AnimMove);
}

float ABMPlayerCharacter::PlayJumpStartOnce(float PlayRate)
{
    return PlayOnce(AnimJumpStart, PlayRate);
}

float ABMPlayerCharacter::PlayAttackOnce(const FBMPlayerAttackSpec& Spec)
{
    return PlayOnce(Spec.Anim, Spec.PlayRate, Spec.StartTime, Spec.MaxPlayTime);
}

float ABMPlayerCharacter::PlayHitOnce(const FBMDamageInfo& Info)
{
    UAnimSequence* Seq = BMCombatUtils::IsHeavyIncoming(Info) ? (AnimHitHeavy ? AnimHitHeavy : AnimHitLight)
        : (AnimHitLight ? AnimHitLight : AnimHitHeavy);
    return PlayOnce(Seq, 1.0f);
}

float ABMPlayerCharacter::PlayDeathOnce()
{
    return PlayOnce(AnimDeath, 1.0f);
}

float ABMPlayerCharacter::PlayDodgeOnce()
{
    return PlayOnce(AnimDodge, DodgePlayRate, 0.0, 0.7);
}


FVector ABMPlayerCharacter::ComputeDodgeDirectionLocked() const
{
    // 优先按当前输入方向闪避
    FVector Dir = GetActorForwardVector();

    if (HasMoveIntent() && Controller)
    {
        const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
        const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

        FVector Wish = Forward * MoveIntent.X + Right * MoveIntent.Y;
        Wish.Z = 0.f;
        if (!Wish.IsNearlyZero())
        {
            Dir = Wish.GetSafeNormal();
        }
    }

    Dir.Z = 0.f;
    return Dir.IsNearlyZero() ? GetActorForwardVector() : Dir.GetSafeNormal();
}

void ABMPlayerCharacter::PlayFallLoop()
{
    PlayLoop(AnimFallLoop);
}

void ABMPlayerCharacter::HandleDamageTaken(const FBMDamageInfo& FinalInfo)
{
    Super::HandleDamageTaken(FinalInfo);

    LastDamageInfo = FinalInfo;

    if (UBMStatsComponent* S = GetStats())
    {
        if (S->IsDead())
        {
            return;
        }
    }

    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;

    const FName Cur = Machine->GetCurrentStateName();

    // 非攻击：必进 Hit
    if (Cur != BMStateNames::Attack)
    {
        Machine->ChangeStateByName(BMStateNames::Hit);
        return;
    }

    // 攻击中：按当前招式规则决定是否打断
    if (ShouldInterruptCurrentAttack(FinalInfo))
    {
        Machine->ChangeStateByName(BMStateNames::Hit);
    }
    // 否则：只扣血，不切状态、不播受击
}

void ABMPlayerCharacter::HandleDeath(const FBMDamageInfo& LastHitInfo)
{
    LastDamageInfo = LastHitInfo;

    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        Machine->ChangeStateByName(BMStateNames::Death);
    }

    Super::HandleDeath(LastHitInfo);
}

bool ABMPlayerCharacter::ResolveHitBoxWindow(
    FName WindowId,
    TArray<FName>& OutHitBoxNames,
    FBMHitBoxActivationParams& OutParams
) const
{
    OutHitBoxNames.Reset();
    OutParams = FBMHitBoxActivationParams();

    if (!bHasActiveAttackSpec)
    {
        return false;
    }

    // 约定：默认窗口名就是 "HitWindow"
    static const FName DefaultWindowId(TEXT("HitWindow"));

    if (WindowId.IsNone() || WindowId == DefaultWindowId)
    {
        OutHitBoxNames = ActiveAttackSpec.HitBoxNames;
        OutParams = ActiveAttackSpec.HitBoxParams;
        return OutHitBoxNames.Num() > 0;
    }

    // 如果你后续升级到多窗口结构（Spec.Windows），在这里再补分支查找 WindowId
    return false;
}