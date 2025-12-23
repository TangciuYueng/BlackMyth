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

#include "InputCoreTypes.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Character/Components/BMHurtBoxComponent.h"
#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMInventoryComponent.h"

#include "UObject/ConstructorHelpers.h"
#include "Core/BMDataSubsystem.h"

#include "Animation/AnimSingleNodeInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "BMGameInstance.h"


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
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttack1Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_A_Slow_MSA.Primary_Melee_A_Slow_MSA'")
		);
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttack2Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_B_Slow_MSA.Primary_Melee_B_Slow_MSA'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttack3Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_C_Slow_MSA.Primary_Melee_C_Slow_MSA'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttack4Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_D_Slow_MSA.Primary_Melee_D_Slow_MSA'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttackRecover1Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_A_Slow_Recovery.Primary_Melee_A_Slow_Recovery'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttackRecover2Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_B_Slow_Recovery.Primary_Melee_B_Slow_Recovery'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttackRecover3Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_C_Slow_Recovery.Primary_Melee_C_Slow_Recovery'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> NormalAttackRecover4Finder(
            TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/useful/Primary_Melee_D_Slow_REcovery.Primary_Melee_D_Slow_REcovery'")
        );
        static ConstructorHelpers::FObjectFinder<UAnimSequence> Skill1Finder(
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

        if (IdleFinder.Succeeded())                     AnimIdle = IdleFinder.Object;
        if (MoveFinder.Succeeded())                     AnimMove = MoveFinder.Object;
		if (NormalAttack1Finder.Succeeded())            AnimNormalAttack1 = NormalAttack1Finder.Object;
		if (NormalAttack2Finder.Succeeded())            AnimNormalAttack2 = NormalAttack2Finder.Object;
		if (NormalAttack3Finder.Succeeded())            AnimNormalAttack3 = NormalAttack3Finder.Object;
		if (NormalAttack4Finder.Succeeded())            AnimNormalAttack4 = NormalAttack4Finder.Object;
		if (NormalAttackRecover1Finder.Succeeded())     AnimNormalAttackRecover1 = NormalAttackRecover1Finder.Object;
		if (NormalAttackRecover2Finder.Succeeded())     AnimNormalAttackRecover2 = NormalAttackRecover2Finder.Object;
		if (NormalAttackRecover3Finder.Succeeded())     AnimNormalAttackRecover3 = NormalAttackRecover3Finder.Object;
		if (NormalAttackRecover4Finder.Succeeded())     AnimNormalAttackRecover4 = NormalAttackRecover4Finder.Object;
		if (Skill1Finder.Succeeded())                   AnimSkill1 = Skill1Finder.Object;
        if (JumpStartFinder.Succeeded())                AnimJumpStart = JumpStartFinder.Object;
        if (FallLoopFinder.Succeeded())                 AnimFallLoop = FallLoopFinder.Object;
        if (HitLightFinder.Succeeded())                 AnimHitLight = HitLightFinder.Object;
        if (HitHeavyFinder.Succeeded())                 AnimHitHeavy = HitHeavyFinder.Object;
        if (DeathFinder.Succeeded())                    AnimDeath = DeathFinder.Object;
        if (DodgeFinder.Succeeded())                    AnimDodge = DodgeFinder.Object;
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

    UBMHurtBoxComponent* Abdomen = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Abdomen"));
    Abdomen->AttachSocketOrBone = TEXT("spine_01");
    Abdomen->BoxExtent = FVector(20, 20, 20);
    Abdomen->RelativeTransform = FTransform(
        FRotator::ZeroRotator,
        FVector(-20.f, 0.f, 0.f),   
        FVector::OneVector
    );
    Abdomen->DamageMultiplier = 0.7f;

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
    BuildAttackSteps();
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

void ABMPlayerCharacter::BuildAttackSteps()
{
    NormalComboSteps.Reset();
    
    {
        FBMPlayerComboStep Step;
        Step.Id = TEXT("Normal_01");
        Step.Anim = AnimNormalAttack1;
        Step.PlayRate = 1.5f;
        Step.LinkWindowSeconds = 0.30f;

        Step.HitBoxNames = { TEXT("LightAttack") };
        Step.HitBoxParams.bOverrideReaction = true;
        Step.HitBoxParams.OverrideReaction = EBMHitReaction::Light;
        Step.RecoverAnim = AnimNormalAttackRecover1;
        Step.RecoverPlayRate = 2.5f;

        Step.bUninterruptible = false;
        Step.InterruptChance = 0.25f;
        Step.InterruptChanceOnHeavyHit = 0.8f;

        NormalComboSteps.Add(Step);
    }
    {
        FBMPlayerComboStep Step;
        Step.Id = TEXT("Normal_02");
        Step.Anim = AnimNormalAttack2;
        Step.PlayRate = 1.3f;
        Step.LinkWindowSeconds = 0.30f;

        Step.HitBoxNames = { TEXT("LightAttack") };
        Step.HitBoxParams.bOverrideReaction = true;
        Step.HitBoxParams.OverrideReaction = EBMHitReaction::Light;
		Step.RecoverAnim = AnimNormalAttackRecover2;
        Step.RecoverPlayRate = 2.5f;

        Step.bUninterruptible = false;
        Step.InterruptChance = 0.25f;
        Step.InterruptChanceOnHeavyHit = 0.8f;

        NormalComboSteps.Add(Step);
    }
    {
        FBMPlayerComboStep Step;
        Step.Id = TEXT("Normal_03");
        Step.Anim = AnimNormalAttack3;
        Step.PlayRate = 1.3f;
        Step.LinkWindowSeconds = 0.30f;

        Step.HitBoxNames = { TEXT("LightAttack") };
        Step.HitBoxParams.bOverrideReaction = true;
        Step.HitBoxParams.OverrideReaction = EBMHitReaction::Light;
        Step.RecoverAnim = AnimNormalAttackRecover3;
        Step.RecoverPlayRate = 2.5f;

        Step.bUninterruptible = false;
        Step.InterruptChance = 0.25f;
        Step.InterruptChanceOnHeavyHit = 0.8f;

        NormalComboSteps.Add(Step);
    }
    {
        FBMPlayerComboStep Step;
        Step.Id = TEXT("Normal_04");
        Step.Anim = AnimNormalAttack4;
        Step.PlayRate = 1.0f;
        Step.LinkWindowSeconds = 0.30f;

        Step.HitBoxNames = { TEXT("LightAttack") };
        Step.HitBoxParams.bOverrideReaction = true;
        Step.HitBoxParams.OverrideReaction = EBMHitReaction::Light;
        Step.RecoverAnim = AnimNormalAttackRecover4;
        Step.RecoverPlayRate = 2.5f;

        Step.bUninterruptible = false;
        Step.InterruptChance = 0.25f;
        Step.InterruptChanceOnHeavyHit = 0.8f;

        NormalComboSteps.Add(Step);
    }

    // 收招动画
    AnimComboRecover = AnimIdle; 

    SkillSlots.Reset();
    {
        FBMPlayerSkillSlot Slot;
        Slot.Action = EBMCombatAction::Skill1;

        Slot.Spec.Id = TEXT("Skill1");
        Slot.Spec.Anim = AnimSkill1;
        Slot.Spec.PlayRate = 1.0f;
        Slot.Spec.StartTime = 0.0f;
        Slot.Spec.MaxPlayTime = -1.0f;

        Slot.Spec.HitBoxNames = { TEXT("HeavyAttack") };
        Slot.Spec.HitBoxParams.bOverrideReaction = true;
        Slot.Spec.HitBoxParams.OverrideReaction = EBMHitReaction::Heavy;
        Slot.Spec.HitBoxParams.DamageMultiplier = 1.25f;

        Slot.Spec.bUninterruptible = true;
        Slot.Spec.InterruptChance = 0.f;
        Slot.Spec.InterruptChanceOnHeavyHit = 0.f;

        Slot.Spec.Cooldown = 2.0f;

        SkillSlots.Add(Slot);
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
    PlayerInputComponent->BindAction(TEXT("NormalAttack"), IE_Pressed, this, &ABMPlayerCharacter::Input_NormalAttackPressed);
    PlayerInputComponent->BindAction(TEXT("Skill1"), IE_Pressed, this, &ABMPlayerCharacter::Input_Skill1Pressed);
    PlayerInputComponent->BindAction(TEXT("Dodge"), IE_Pressed, this, &ABMPlayerCharacter::Input_DodgePressed);

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

void ABMPlayerCharacter::Input_NormalAttackPressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::NormalAttack);
    }
}

void ABMPlayerCharacter::Input_Skill1Pressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::Skill1);
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
    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;
    const FName CurState = Machine->GetCurrentStateName();
    // 普通攻击
    if (Action == EBMCombatAction::NormalAttack)
    {
        EnqueueAction(Action);
        if (CurState != BMStateNames::Attack)
        {
            if (UCharacterMovementComponent* Move = GetCharacterMovement())
            {
                if (Move->IsFalling()) return;
            }
            Machine->ChangeStateByName(BMStateNames::Attack);
        }
        return;
    }

    // 技能：单段，要求冷却就绪
    if (BMCombatUtils::IsSkillAction(Action))
    {
        if (CurState == BMStateNames::Attack || CurState == BMStateNames::Dodge)
        {
            return; // 不允许攻击中插技能
        }

        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            if (Move->IsFalling()) return;
        }

        FBMPlayerAttackSpec Spec;
        if (!SelectSkillSpec(Action, Spec)) return;

        if (UBMCombatComponent* C = GetCombat())
        {
            if (!C->IsCooldownReady(Spec.Id))
                return;
        }

        EnqueueAction(Action);
        Machine->ChangeStateByName(BMStateNames::Attack);
        return;
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

        if (UBMStateMachineComponent* TempMachine = GetFSM())
        {
            TempMachine->ChangeStateByName(BMStateNames::Dodge);
        }
        return;
    }

}

bool ABMPlayerCharacter::ShouldInterruptCurrentAttack(const FBMDamageInfo& Incoming) const
{
    if (!bHasActiveAttackContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] No ActiveAttackContext while attacking."), *GetName());
        return true;
    }


    if (bActiveUninterruptible) return false;

    float P = BMCombatUtils::IsHeavyIncoming(Incoming) ? ActiveInterruptChanceOnHeavyHit : ActiveInterruptChance;
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

float ABMPlayerCharacter::PlayNormalAttackOnce(
    UAnimSequence* Seq,
    float PlayRate,
    float StartTime,
    float MaxPlayTime)
{
	return PlayOnce(Seq, PlayRate, StartTime, MaxPlayTime);
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

float ABMPlayerCharacter::PlayComboRecoverOnce(const FBMPlayerComboStep& Step)
{
    if (!Step.RecoverAnim)
    {
        return 0.f;
    }

    return PlayOnce(
        Step.RecoverAnim,
        Step.RecoverPlayRate,
        Step.RecoverStartTime,
        Step.RecoverMaxPlayTime
    );
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

    // Stop any level music via GameInstance
    if (UWorld* World = GetWorld())
    {
        if (UBMGameInstance* GI = Cast<UBMGameInstance>(World->GetGameInstance()))
        {
            GI->StopLevelMusic();
        }
    }

    // Play death music
    static const TCHAR* DeathSoundPath = TEXT("/Game/Audio/death.death");
    if (UWorld* World = GetWorld())
    {
        if (UBMGameInstance* GI = Cast<UBMGameInstance>(World->GetGameInstance()))
        {
            GI->PlayMusic(World, DeathSoundPath, /*bLoop=*/false);
        }
        else if (USoundBase* DeathSound = LoadObject<USoundBase>(nullptr, DeathSoundPath))
        {
            UGameplayStatics::PlaySound2D(World, DeathSound);
        }
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

    if (!bHasActiveAttackContext)
    {
        return false;
    }

    // 默认窗口名就是 "HitWindow"
    static const FName DefaultWindowId(TEXT("HitWindow"));

    if (WindowId.IsNone() || WindowId == DefaultWindowId)
    {
        OutHitBoxNames = ActiveHitBoxNames;
        OutParams = ActiveHitBoxParams;
        return OutHitBoxNames.Num() > 0;
    }

    return false;
}

void ABMPlayerCharacter::EnqueueAction(EBMCombatAction Action)
{
    ActionQueue.Add(Action);
}

bool ABMPlayerCharacter::ConsumeNextQueuedAction(EBMCombatAction& OutAction)
{
    if (ActionQueue.Num() <= 0) return false;
    OutAction = ActionQueue[0];
    ActionQueue.RemoveAt(0);
    return true;
}

bool ABMPlayerCharacter::ConsumeOneQueuedNormalAttack()
{
    for (int32 i = 0; i < ActionQueue.Num(); ++i)
    {
        if (ActionQueue[i] == EBMCombatAction::NormalAttack)
        {
            ActionQueue.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool ABMPlayerCharacter::SelectSkillSpec(EBMCombatAction Action, FBMPlayerAttackSpec& OutSpec) const
{
    for (const FBMPlayerSkillSlot& S : SkillSlots)
    {
        if (S.Action == Action)
        {
            OutSpec = S.Spec;
            return (OutSpec.Anim != nullptr);
        }
    }
    return false;
}

bool ABMPlayerCharacter::GetComboStep(int32 Index, FBMPlayerComboStep& Out) const
{
    if (!NormalComboSteps.IsValidIndex(Index)) return false;
    Out = NormalComboSteps[Index];
    return Out.Anim != nullptr;
}

void ABMPlayerCharacter::SetActiveAttackContext(
    const TArray<FName>& HitBoxNames,
    const FBMHitBoxActivationParams& Params,
    bool bUninterruptible,
    float InterruptChance,
    float InterruptChanceOnHeavyHit
)
{
    ActiveHitBoxNames = HitBoxNames;
    ActiveHitBoxParams = Params;

    bActiveUninterruptible = bUninterruptible;
    ActiveInterruptChance = InterruptChance;
    ActiveInterruptChanceOnHeavyHit = InterruptChanceOnHeavyHit;

    bHasActiveAttackContext = true;
}

void ABMPlayerCharacter::ClearActiveAttackContext()
{
    bHasActiveAttackContext = false;
    ActiveHitBoxNames.Reset();
    ActiveHitBoxParams = FBMHitBoxActivationParams();

    bActiveUninterruptible = false;
    ActiveInterruptChance = 0.f;
    ActiveInterruptChanceOnHeavyHit = 0.f;
}

