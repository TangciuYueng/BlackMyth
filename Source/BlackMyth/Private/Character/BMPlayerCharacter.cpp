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

#include "InputCoreTypes.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Character/Components/BMHurtBoxComponent.h"
#include "Character/Components/BMHitBoxComponent.h"

#include "Character/Components/BMInventoryComponent.h"

#include "Core/BMDataSubsystem.h"

#include "Animation/AnimSingleNodeInstance.h"


ABMPlayerCharacter::ABMPlayerCharacter()
{
    CharacterType = EBMCharacterType::Player;
    Team = EBMTeam::Player;

    Inventory = CreateDefaultSubobject<UBMInventoryComponent>(TEXT("Inventory"));

	// Scheme B: set a default inventory UI widget class in pure C++.
	// Note: update this path to match your widget blueprint asset if different.
	static ConstructorHelpers::FClassFinder<UUserWidget> InventoryWidgetClassFinder(
		TEXT("/Game/UI/WBP_Inventory")
	);
	if (InventoryWidgetClassFinder.Succeeded() && Inventory)
	{
		Inventory->SetInventoryWidgetClass(InventoryWidgetClassFinder.Class);
	}

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

    // === HurtBoxes ===
    UBMHurtBoxComponent* Head = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Head"));
    Head->AttachSocketOrBone = TEXT("head");
    Head->BoxExtent = FVector(10, 10, 10);
    Head->DamageMultiplier = 1.6f; // 头部更疼

    UBMHurtBoxComponent* Body = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Body"));
    Body->AttachSocketOrBone = TEXT("spine_03");
    Body->BoxExtent = FVector(14, 18, 20);
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
    }
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

    PlayerInputComponent->BindKey(EKeys::I, IE_Pressed, this, &ABMPlayerCharacter::Input_ToggleInventory);
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot1);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot2);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot3);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot4);
	PlayerInputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot5);
	PlayerInputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot6);
	PlayerInputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot7);
	PlayerInputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot8);
	PlayerInputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ABMPlayerCharacter::HotbarSlot9);
	PlayerInputComponent->BindKey(EKeys::M, IE_Pressed, this, &ABMPlayerCharacter::Input_ToggleAutoAddCurrencyTest);
	PlayerInputComponent->BindKey(EKeys::N, IE_Pressed, this, &ABMPlayerCharacter::Input_ToggleForcePrice10Test);
}

void ABMPlayerCharacter::Input_ToggleInventory()
{
    if (Inventory)
    {
        Inventory->ToggleInventoryUI();
    }
}

void ABMPlayerCharacter::Input_ToggleAutoAddCurrencyTest()
{
	if (Inventory)
	{
		Inventory->ToggleTestAutoAddCurrency();
	}
}

void ABMPlayerCharacter::Input_ToggleForcePrice10Test()
{
	if (Inventory)
	{
		Inventory->ToggleTestForceItemPrice10();
	}
}

void ABMPlayerCharacter::HotbarSlot1() { TriggerHotbarSlot(1); }
void ABMPlayerCharacter::HotbarSlot2() { TriggerHotbarSlot(2); }
void ABMPlayerCharacter::HotbarSlot3() { TriggerHotbarSlot(3); }
void ABMPlayerCharacter::HotbarSlot4() { TriggerHotbarSlot(4); }
void ABMPlayerCharacter::HotbarSlot5() { TriggerHotbarSlot(5); }
void ABMPlayerCharacter::HotbarSlot6() { TriggerHotbarSlot(6); }
void ABMPlayerCharacter::HotbarSlot7() { TriggerHotbarSlot(7); }
void ABMPlayerCharacter::HotbarSlot8() { TriggerHotbarSlot(8); }
void ABMPlayerCharacter::HotbarSlot9() { TriggerHotbarSlot(9); }

void ABMPlayerCharacter::TriggerHotbarSlot(int32 SlotIndex)
{
	if (!Inventory)
	{
		return;
	}

	static const FName HotbarItemIDs[9] = {
		TEXT("Item_CheDianWei"),
		TEXT("Item_ChuBaiQiangTou"),
		TEXT("Item_JinChenXin"),
		TEXT("Item_JinGuangYanMou"),
		TEXT("Item_JiuZhuanJinDan"),
		TEXT("Item_TaiYiZiJinDan"),
		TEXT("Item_TieShiXin"),
		TEXT("Item_YaoShengJiao"),
		TEXT("Item_YinXingWuJiao")
	};

	if (SlotIndex < 1 || SlotIndex > 9)
	{
		return;
	}

	const FName ItemID = HotbarItemIDs[SlotIndex - 1];
	UE_LOG(LogTemp, Log, TEXT("Hotbar slot %d -> %s"), SlotIndex, *ItemID.ToString());

	if (Inventory->IsInventoryUIVisible())
	{
		const float UnitPrice = Inventory->GetItemPrice(ItemID);
		const int32 UnitCost = FMath::Max(0, FMath::RoundToInt(UnitPrice));
		const bool bCanAfford = (UnitCost == 0) || Inventory->CanAfford(UnitCost);
		if (!bCanAfford)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hotbar slot %d purchase blocked: need %d, have %d"), SlotIndex, UnitCost, Inventory->GetCurrency());
			return;
		}

		if (!Inventory->AddItem(ItemID, 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("Hotbar slot %d purchase failed: AddItem(%s) returned false (check ItemDataTable row exists and capacity)"), SlotIndex, *ItemID.ToString());
			return;
		}

		if (UnitCost > 0)
		{
			Inventory->SpendCurrency(UnitCost);
		}
	}
	else
	{
		Inventory->UseItem(ItemID, 1);
	}
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

float ABMPlayerCharacter::PlayLightAttackOnce(float PlayRate)
{
    return PlayOnce(AnimLightAttack, PlayRate, 0.0, 0.3);
}

float ABMPlayerCharacter::PlayJumpStartOnce(float PlayRate)
{
    return PlayOnce(AnimJumpStart, PlayRate);
}

void ABMPlayerCharacter::PlayFallLoop()
{
    PlayLoop(AnimFallLoop);
}
