#include "Character/BMPlayerCharacter.h"

#include "Character/Components/BMCombatComponent.h"
#include "Character/Components/BMHurtBoxComponent.h"
#include "Character/Components/BMHitBoxComponent.h"
#include "Character/Components/BMStatsComponent.h"
#include "Character/Components/BMInventoryComponent.h"
#include "Character/Components/BMExperienceComponent.h"
#include "Character/Components/BMStateMachineComponent.h"
#include "Character/States/BMPlayerState_Idle.h"
#include "Character/States/BMPlayerState_Move.h"
#include "Character/States/BMPlayerState_Jump.h"
#include "Character/States/BMPlayerState_Attack.h"
#include "Character/States/BMPlayerState_Hit.h"
#include "Character/States/BMPlayerState_Death.h"
#include "Character/States/BMPlayerState_Dodge.h"
#include "System/Save/BMSaveGameSubsystem.h"
#include "System/Event/BMEventBusSubsystem.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Core/BMTypes.h"

#include "InputCoreTypes.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "UObject/ConstructorHelpers.h"
#include "Core/BMDataSubsystem.h"

#include "Animation/AnimSingleNodeInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "BMGameInstance.h"
#include "Engine/GameInstance.h"

/*
 * @brief Constructor of the ABMPlayerCharacter class
 */
ABMPlayerCharacter::ABMPlayerCharacter()
{
    CharacterType = EBMCharacterType::Player;
    Team = EBMTeam::Player;

    Inventory = CreateDefaultSubobject<UBMInventoryComponent>(TEXT("Inventory"));
	static ConstructorHelpers::FClassFinder<UUserWidget> InventoryWidgetClassFinder(
		TEXT("/Game/UI/WBP_Inventory")
	);
	if (InventoryWidgetClassFinder.Succeeded() && Inventory)
	{
		Inventory->SetInventoryWidgetClass(InventoryWidgetClassFinder.Class);
	}

    Experience = CreateDefaultSubobject<UBMExperienceComponent>(TEXT("ExperienceComponent"));
    if (Experience)
    {
        Experience->SetIsReplicated(false); 
    }

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = true;
        Move->RotationRate = FRotator(0.f, 720.f, 0.f);
        Move->JumpZVelocity = 600.f;
        Move->AirControl = 0.25f;
    }

    SetupCamera();
    SetupMesh();
    SetupAnimations();
    SetupHurtBoxes();
    SetupHitBoxes();
    
    BuildAttackSteps();
}

/*
 * @brief Setup camera, it setups the camera
 */
void ABMPlayerCharacter::SetupCamera()
{
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 350.f;
    CameraBoom->bUsePawnControlRotation = true;   
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.f;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->ProbeChannel = ECC_GameTraceChannel1;
    CameraBoom->ProbeSize = 12.f;
    CameraBoom->bUseCameraLagSubstepping = true;
    CameraBoom->CameraLagMaxTimeStep = 1.f / 60.f;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

/*
 * @brief Setup mesh, it setups the mesh
 */
void ABMPlayerCharacter::SetupMesh()
{
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
        TEXT("/Script/Engine.SkeletalMesh'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Meshes/Wukong.Wukong'")
    );
    if (MeshFinder.Succeeded())
    {
        CharacterMeshAsset = MeshFinder.Object;
        GetMesh()->SetSkeletalMesh(CharacterMeshAsset);
    }

    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
}

/*
 * @brief Setup animations, it setups the animations
 */
void ABMPlayerCharacter::SetupAnimations()
{
    static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleFinder(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Idle_AO_CC.Idle_AO_CC'")
    );
    static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkFinder(
        TEXT("/Script/Engine.AnimSequence'/Game/Stylized_Spruce_Forest/Demo/Game_Mode/Mannequin/Animations/ThirdPersonWalk.ThirdPersonWalk'")
    );
    static ConstructorHelpers::FObjectFinder<UAnimSequence> RunFinder(
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
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Skill2Finder(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/Primary_Melee_E_Slow.Primary_Melee_E_Slow'")
    );
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Skill3Finder(
        TEXT("/Script/Engine.AnimSequence'/Game/ParagonSunWukong/Characters/Heroes/Wukong/Animations/RMB_Push.RMB_Push'")
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
    if (WalkFinder.Succeeded())                     AnimWalk = WalkFinder.Object;
    if (RunFinder.Succeeded())                      AnimRun = RunFinder.Object;
    if (NormalAttack1Finder.Succeeded())            AnimNormalAttack1 = NormalAttack1Finder.Object;
    if (NormalAttack2Finder.Succeeded())            AnimNormalAttack2 = NormalAttack2Finder.Object;
    if (NormalAttack3Finder.Succeeded())            AnimNormalAttack3 = NormalAttack3Finder.Object;
    if (NormalAttack4Finder.Succeeded())            AnimNormalAttack4 = NormalAttack4Finder.Object;
    if (NormalAttackRecover1Finder.Succeeded())     AnimNormalAttackRecover1 = NormalAttackRecover1Finder.Object;
    if (NormalAttackRecover2Finder.Succeeded())     AnimNormalAttackRecover2 = NormalAttackRecover2Finder.Object;
    if (NormalAttackRecover3Finder.Succeeded())     AnimNormalAttackRecover3 = NormalAttackRecover3Finder.Object;
    if (NormalAttackRecover4Finder.Succeeded())     AnimNormalAttackRecover4 = NormalAttackRecover4Finder.Object;
    if (Skill1Finder.Succeeded())                   AnimSkill1 = Skill1Finder.Object;
    if (Skill2Finder.Succeeded())                   AnimSkill2 = Skill2Finder.Object;
    if (Skill3Finder.Succeeded())                   AnimSkill3 = Skill3Finder.Object;
    if (JumpStartFinder.Succeeded())                AnimJumpStart = JumpStartFinder.Object;
    if (FallLoopFinder.Succeeded())                 AnimFallLoop = FallLoopFinder.Object;
    if (HitLightFinder.Succeeded())                 AnimHitLight = HitLightFinder.Object;
    if (HitHeavyFinder.Succeeded())                 AnimHitHeavy = HitHeavyFinder.Object;
    if (DeathFinder.Succeeded())                    AnimDeath = DeathFinder.Object;
    if (DodgeFinder.Succeeded())                    AnimDodge = DodgeFinder.Object;
}

/*
 * @brief Setup hurt boxes, it setups the hurt boxes
 */
void ABMPlayerCharacter::SetupHurtBoxes()
{
    UBMHurtBoxComponent* Head = CreateDefaultSubobject<UBMHurtBoxComponent>(TEXT("HB_Head"));
    Head->AttachSocketOrBone = TEXT("head");
    Head->BoxExtent = FVector(20, 20, 20);
    Head->DamageMultiplier = 1.6f;

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
}

/*
 * @brief Setup hit boxes, it setups the hit boxes
 */
void ABMPlayerCharacter::SetupHitBoxes()
{
    if (UBMHitBoxComponent* HB = GetHitBox())
    {
        FBMHitBoxDefinition Light;
        Light.Name = TEXT("LightAttack");
        Light.Type = EBMHitBoxType::LightAttack;
        Light.AttachSocketOrBone = TEXT("weapon_r");  
        Light.BoxExtent = FVector(8, 130, 8);
        Light.DamageType = EBMDamageType::Melee;
        Light.DamageScale = 1.0f;
        Light.AdditiveDamage = 0.f;
        Light.DefaultReaction = EBMHitReaction::Light;

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
        Heavy.DamageScale = 1.35f;
        Heavy.AdditiveDamage = 0.f;
        Heavy.DefaultReaction = EBMHitReaction::Heavy;

        HB->RegisterDefinition(Heavy);
    }
}

/*
 * @brief Begin play, it begins the play
 */
void ABMPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitFSMStates();

    // �� Combat �¼�
    if (UBMCombatComponent* C = GetCombat())
    {
        C->OnActionRequested.AddUObject(this, &ABMPlayerCharacter::OnActionRequested);
    }
    bSprintHeld = false;
    ApplyGait();
    // ��ʼ����
    PlayIdleLoop();

	// ���ԣ����� HitBox/HurtBox ���ӻ�
    //if (UBMHitBoxComponent* HB = GetHitBox()) HB->bDebugDraw = true;
    //for (UBMHurtBoxComponent* HB : HurtBoxes)
    //{
    //    if (!HB) continue;
    //    HB->bDebugDraw = true;
    //}

    // ===== �л��ؿ����Զ����ش浵 =====
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UBMSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<UBMSaveGameSubsystem>())
            {
                // ����Ƿ�����Զ��浵����λ0��
                if (SaveSystem->DoesSaveExist(0))
                {
                    UE_LOG(LogTemp, Warning, TEXT("[BMPlayerCharacter] Auto-save found, loading player data (without position)..."));
                    
                    FTimerHandle LoadDelayTimer;
                    World->GetTimerManager().SetTimerForNextTick([this, SaveSystem]()
                    {
                        // �����Զ��浵�����ָ�λ�ã�
                        bool bLoadSuccess = SaveSystem->LoadGame(0, false);
                        
                        if (bLoadSuccess)
                        {
                            UE_LOG(LogTemp, Warning, TEXT("[BMPlayerCharacter] Player data loaded successfully (position from PlayerStart)!"));
                            
                            // ���غ�����ɾ���Զ��浵
                            SaveSystem->DeleteSave(0);
                            UE_LOG(LogTemp, Warning, TEXT("[BMPlayerCharacter] Auto-save deleted (only for level transition)."));
                        }
                        else
                        {
                            UE_LOG(LogTemp, Error, TEXT("[BMPlayerCharacter] Failed to load player data!"));
                        }
                    });
                }
                else
                {                   
                    UE_LOG(LogTemp, Log, TEXT("[BMPlayerCharacter] No auto-save found. Using current state (Level: %d)."), 
                        Experience ? Experience->GetLevel() : 1);
                }
            }
        }
    }
}

/*
 * @brief Build attack steps, it builds the attack steps
 */
void ABMPlayerCharacter::BuildAttackSteps()
{
    NormalComboSteps.Reset();
    
    {
        FBMPlayerComboStep Step;
        Step.Id = TEXT("Normal_01");
        Step.Anim = AnimNormalAttack1;
        Step.PlayRate = 1.5f;
        Step.LinkWindowSeconds = 0.50f;

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
        Step.LinkWindowSeconds = 0.50f;

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
        Step.LinkWindowSeconds = 0.50f;

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
        Step.LinkWindowSeconds = 0.50f;

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

    SkillSlots.Reset();
    {
        FBMPlayerSkillSlot Slot;
        Slot.Action = EBMCombatAction::Skill1;

        Slot.StaminaCost = 30.f;

        Slot.Spec.Id = TEXT("Skill1");
        Slot.Spec.Anim = AnimSkill1;
        Slot.Spec.PlayRate = 1.0f;
        Slot.Spec.StartTime = 0.0f;
        Slot.Spec.MaxPlayTime = -1.0f;

        Slot.Spec.HitBoxNames = { TEXT("HeavyAttack") };
        Slot.Spec.HitBoxParams.bOverrideReaction = true;
        Slot.Spec.HitBoxParams.OverrideReaction = EBMHitReaction::Heavy;
        Slot.Spec.HitBoxParams.DamageMultiplier = 1.5f;

        Slot.Spec.bUninterruptible = true;
        Slot.Spec.InterruptChance = 0.f;
        Slot.Spec.InterruptChanceOnHeavyHit = 0.f;

        Slot.Spec.Cooldown = 2.0f;

        SkillSlots.Add(Slot);
    }
    {
        FBMPlayerSkillSlot Slot;
        Slot.Action = EBMCombatAction::Skill2;

        Slot.StaminaCost = 50.f;

        Slot.Spec.Id = TEXT("Skill2");
        Slot.Spec.Anim = AnimSkill2;
        Slot.Spec.PlayRate = 1.0f;
        Slot.Spec.StartTime = 0.0f;
        Slot.Spec.MaxPlayTime = 0.5f;

        Slot.Spec.HitBoxNames = { TEXT("HeavyAttack") };
        Slot.Spec.HitBoxParams.bOverrideReaction = true;
        Slot.Spec.HitBoxParams.OverrideReaction = EBMHitReaction::Heavy;
        Slot.Spec.HitBoxParams.DamageMultiplier = 1.8f;

        Slot.Spec.bUninterruptible = true;
        Slot.Spec.InterruptChance = 0.f;
        Slot.Spec.InterruptChanceOnHeavyHit = 0.f;

        Slot.Spec.Cooldown = 4.0f;

        SkillSlots.Add(Slot);
    }
    {
        FBMPlayerSkillSlot Slot;
        Slot.Action = EBMCombatAction::Skill3;

        Slot.StaminaCost = 70.f;

        Slot.Spec.Id = TEXT("Skill3");
        Slot.Spec.Anim = AnimSkill3;
        Slot.Spec.PlayRate = 1.0f;
        Slot.Spec.StartTime = 0.0f;
        Slot.Spec.MaxPlayTime = -1.0f;

        Slot.Spec.HitBoxNames = { TEXT("HeavyAttack") };
        Slot.Spec.HitBoxParams.bOverrideReaction = true;
        Slot.Spec.HitBoxParams.OverrideReaction = EBMHitReaction::Heavy;
        Slot.Spec.HitBoxParams.DamageMultiplier = 2.5f;

        Slot.Spec.bUninterruptible = true;
        Slot.Spec.InterruptChance = 0.f;
        Slot.Spec.InterruptChanceOnHeavyHit = 0.f;

        Slot.Spec.Cooldown = 8.0f;

        SkillSlots.Add(Slot);
    }

}

void ABMPlayerCharacter::InitFSMStates()
{
    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;

    // ������ע��״̬
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

    // ��ʼ״̬
    Machine->ChangeStateByName(BMStateNames::Idle);
}

void ABMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // ��ͳ����ӳ��
    // Axis: MoveForward / MoveRight
    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABMPlayerCharacter::Input_MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABMPlayerCharacter::Input_MoveRight);

    // Action: Jump / Attack
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ABMPlayerCharacter::Input_JumpPressed);
    PlayerInputComponent->BindAction(TEXT("NormalAttack"), IE_Pressed, this, &ABMPlayerCharacter::Input_NormalAttackPressed);
    PlayerInputComponent->BindAction(TEXT("Skill1"), IE_Pressed, this, &ABMPlayerCharacter::Input_Skill1Pressed);
    PlayerInputComponent->BindAction(TEXT("Skill2"), IE_Pressed, this, &ABMPlayerCharacter::Input_Skill2Pressed);
    PlayerInputComponent->BindAction(TEXT("Skill3"), IE_Pressed, this, &ABMPlayerCharacter::Input_Skill3Pressed);
    PlayerInputComponent->BindAction(TEXT("Dodge"), IE_Pressed, this, &ABMPlayerCharacter::Input_DodgePressed);

    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &ABMPlayerCharacter::Input_SprintPressed);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &ABMPlayerCharacter::Input_SprintReleased);


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

/*
 * @brief Trigger hotbar slot, it triggers the hotbar slot
 * @param SlotIndex The slot index
 */
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
			// ͨ���¼�����֪ͨ��ҽ�Ҳ���
			if (UWorld* World = GetWorld())
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (UBMEventBusSubsystem* EventBus = GI->GetSubsystem<UBMEventBusSubsystem>())
					{
						FString ItemName = ItemID.ToString();
						ItemName.RemoveFromStart(TEXT("Item_"));
						FString NotificationMessage = FString::Printf(TEXT("Insufficient Currency: %s costs %d, have %d"), 
							*ItemName, UnitCost, Inventory->GetCurrency());
						EventBus->EmitNotify(FText::FromString(NotificationMessage));
					}
				}
			}
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

		// ͨ���¼�����֪ͨ����ɹ�
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				if (UBMEventBusSubsystem* EventBus = GI->GetSubsystem<UBMEventBusSubsystem>())
				{
					FString ItemName = ItemID.ToString();
					ItemName.RemoveFromStart(TEXT("Item_"));
					FString NotificationMessage = FString::Printf(TEXT("Purchase Success: %s (Cost: %d, Remaining: %d)"), 
						*ItemName, UnitCost, Inventory->GetCurrency());
					EventBus->EmitNotify(FText::FromString(NotificationMessage));
				}
			}
		}
	}
	else
	{
		Inventory->UseItem(ItemID, 1);
	}
}

/*
 * @brief Input move forward, it inputs the move forward
 * @param Value The value
 */
void ABMPlayerCharacter::Input_MoveForward(float Value)
{
    MoveIntent.X = Value;
    UpdateMoveIntent();
}

/*
 * @brief Input move right, it inputs the move right
 * @param Value The value
 */
void ABMPlayerCharacter::Input_MoveRight(float Value)
{
    MoveIntent.Y = Value;
    UpdateMoveIntent();
}

void ABMPlayerCharacter::UpdateMoveIntent()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        if (!C->CanPerformAction())
        {
            return;
        }
    }

    if (HasMoveIntent()) ApplyGait();

    if (Controller && HasMoveIntent())
    {
        const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
        const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

        AddMovementInput(Forward, MoveIntent.X);
        AddMovementInput(Right, MoveIntent.Y);
    }
}

void ABMPlayerCharacter::Input_SprintPressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        if (!C->CanPerformAction())
        {
            return;
        }
    }

    bSprintHeld = true;
    ApplyGait();
}

void ABMPlayerCharacter::Input_SprintReleased()
{
    bSprintHeld = false;
    ApplyGait();
}

void ABMPlayerCharacter::ApplyGait()
{
    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (!Move) return;

    // �ٶ��л�
    Move->MaxWalkSpeed = bSprintHeld ? RunSpeed : WalkSpeed;

    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        if (Machine->GetCurrentStateName() == BMStateNames::Move)
        {
            PlayMoveLoop();
        }
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
        return; // ���в������ٴ�����
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

void ABMPlayerCharacter::Input_Skill2Pressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::Skill2);
    }
}

void ABMPlayerCharacter::Input_Skill3Pressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::Skill3);
    }
}

void ABMPlayerCharacter::Input_DodgePressed()
{
    if (UBMCombatComponent* C = GetCombat())
    {
        C->RequestAction(EBMCombatAction::Dodge);
    }
}

/*
 * @brief Input turn, it inputs the turn
 * @param Value The value
 */
void ABMPlayerCharacter::Input_Turn(float Value)
{
    AddControllerYawInput(Value);
}

/*
 * @brief Input look up, it inputs the look up
 * @param Value The value
 */
void ABMPlayerCharacter::Input_LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

/*
 * @brief On action requested, it on action requested
 * @param Action The action
 */
void ABMPlayerCharacter::OnActionRequested(EBMCombatAction Action)
{
    // ��������Ӧ
    if (UBMStatsComponent* S = GetStats())
    {
        if (S->IsDead()) return;
    }
    UBMStateMachineComponent* Machine = GetFSM();
    if (!Machine) return;
    const FName CurState = Machine->GetCurrentStateName();
    // ��ͨ����
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

    // ���ܵ��Σ�Ҫ����ȴ����
    if (BMCombatUtils::IsSkillAction(Action))
    {
        if (CurState == BMStateNames::Attack || CurState == BMStateNames::Dodge)
        {
            return; // �����������в弼��
        }

        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            if (Move->IsFalling()) return;
        }

        FBMPlayerAttackSpec Spec;
        float SkillStaminaCost = 0.f;
        if (!SelectSkillSpec(Action, Spec, SkillStaminaCost)) return;

        if (UBMCombatComponent* C = GetCombat())
        {
            if (!C->IsCooldownReady(Spec.Id))
                return;
        }

        if (UBMStatsComponent* S = GetStats())
        {
            if (S->GetStatBlock().Stamina < SkillStaminaCost)
            {
                return;
            }
        }

        EnqueueAction(Action);
        Machine->ChangeStateByName(BMStateNames::Attack);
        return;
    }

    // ��������������
    if (Action == EBMCombatAction::Dodge)
    {
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            if (Move->IsFalling()) return;
        }

        // ��ȴ���
        if (UBMCombatComponent* C = GetCombat())
        {
            if (!C->IsCooldownReady(DodgeCooldownKey))
            {
                return;
            }
        }

        if (UBMStatsComponent* S = GetStats())
        {
            if (S->GetStatBlock().Stamina < 40.f)
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

/*
 * @brief Should interrupt current attack, it should interrupt the current attack
 * @param Incoming The incoming
 * @return True if the current attack should be interrupted, false otherwise
 */
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

/*
 * @brief Landed, it landed
 * @param Hit The hit
 */
void ABMPlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    bPendingJump = false;
    // ��غ�ص� Move/Idle
    if (UBMStateMachineComponent* Machine = GetFSM())
    {
        Machine->ChangeStateByName(HasMoveIntent() ? BMStateNames::Move : BMStateNames::Idle);
    }
}

/*
 * @brief Play loop, it plays the loop
 * @param Seq The sequence
 */

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

/*
 * @brief Play once, it plays the once
 * @param Seq The sequence
 * @param PlayRate The play rate
 * @param StartTime The start time
 * @param MaxPlayTime The max play time
 * @return The play time
 */
float ABMPlayerCharacter::PlayOnce(UAnimSequence* Seq, float PlayRate, float StartTime, float MaxPlayTime)
{
    if (!Seq || !GetMesh())
    {
        return 0.f;
    }

    CurrentLoopAnim = nullptr;

    // ȷ���ǵ��ڵ�ģʽ
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    // �� SingleNodeInstance ������ʼʱ��/������
    UAnimSingleNodeInstance* Inst = GetMesh()->GetSingleNodeInstance();

    // ��һ��û���� Instance���� PlayAnimation һ��
    if (!Inst)
    {
        GetMesh()->PlayAnimation(Seq, false);
        Inst = GetMesh()->GetSingleNodeInstance();
    }

    const float Len = Seq->GetPlayLength();
    const float SafePlayRate = (PlayRate > 0.f) ? PlayRate : 1.0f;

    const float ClampedStart = FMath::Clamp(StartTime, 0.f, Len);
    const float Remaining = FMath::Max(0.f, Len - ClampedStart);

    // MaxPlayTime <= 0 ��ʾ������ʣ��Σ�����ü�
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
        // ���޷�������ʼʱ��/�ü�
        GetMesh()->PlayAnimation(Seq, false);
        return Len / SafePlayRate;
    }

    // ����ʱ��
    return EffectivePlayTime / SafePlayRate;
}

/*
 * @brief Play idle loop, it plays the idle loop
 */
void ABMPlayerCharacter::PlayIdleLoop()
{
    PlayLoop(AnimIdle);
}

/*
 * @brief Play move loop, it plays the move loop
 */
void ABMPlayerCharacter::PlayMoveLoop()
{
	UAnimSequence* Seq = bSprintHeld ? AnimRun : AnimWalk;
	PlayLoop(Seq);
}

/*
 * @brief Play jump start once, it plays the jump start once
 * @param PlayRate The play rate
 * @return The play time
 */
float ABMPlayerCharacter::PlayJumpStartOnce(float PlayRate)
{
    return PlayOnce(AnimJumpStart, PlayRate);
}

/*
 * @brief Play attack once, it plays the attack once
 * @param Spec The spec
 * @return The play time
 */
float ABMPlayerCharacter::PlayAttackOnce(const FBMPlayerAttackSpec& Spec)
{
    return PlayOnce(Spec.Anim, Spec.PlayRate, Spec.StartTime, Spec.MaxPlayTime);
}

/*
 * @brief Play normal attack once, it plays the normal attack once
 * @param Seq The sequence
 * @param PlayRate The play rate
 * @param StartTime The start time
 * @param MaxPlayTime The max play time
 * @return The play time
 */
float ABMPlayerCharacter::PlayNormalAttackOnce(
    UAnimSequence* Seq,
    float PlayRate,
    float StartTime,
    float MaxPlayTime)
{
	return PlayOnce(Seq, PlayRate, StartTime, MaxPlayTime);
}

/*
 * @brief Play hit once, it plays the hit once
 * @param Info The info
 * @return The play time
 */
float ABMPlayerCharacter::PlayHitOnce(const FBMDamageInfo& Info)
{
    UAnimSequence* Seq = BMCombatUtils::IsHeavyIncoming(Info) ? (AnimHitHeavy ? AnimHitHeavy : AnimHitLight)
        : (AnimHitLight ? AnimHitLight : AnimHitHeavy);
    return PlayOnce(Seq, 1.0f);
}

/*
 * @brief Play death once, it plays the death once
 * @return The play time
 */
float ABMPlayerCharacter::PlayDeathOnce()
{
    return PlayOnce(AnimDeath, 1.0f);
}

/*
 * @brief Play dodge once, it plays the dodge once
 * @return The play time
 */
float ABMPlayerCharacter::PlayDodgeOnce()
{
    return PlayOnce(AnimDodge, DodgePlayRate, 0.0, 0.7);
}

/*
 * @brief Play combo recover once, it plays the combo recover once
 * @param Step The step
 * @return The play time
 */
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

/*
 * @brief Compute dodge direction locked, it computes the dodge direction locked
 * @return The dodge direction locked
 */
FVector ABMPlayerCharacter::ComputeDodgeDirectionLocked() const
{
    // ���Ȱ���ǰ���뷽������
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

/*
 * @brief Play fall loop, it plays the fall loop
 */
void ABMPlayerCharacter::PlayFallLoop()
{
    PlayLoop(AnimFallLoop);
}

/*
 * @brief Handle damage taken, it handles the damage taken
 * @param FinalInfo The final info
 */
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

    // �ǹ����ؽ� Hit
    if (Cur != BMStateNames::Attack)
    {
        Machine->ChangeStateByName(BMStateNames::Hit);
        return;
    }

    // �����а���ǰ��ʽ��������Ƿ���
    if (ShouldInterruptCurrentAttack(FinalInfo))
    {
        Machine->ChangeStateByName(BMStateNames::Hit);
    }
    // ����ֻ��Ѫ������״̬�������ܻ�
}

/*
 * @brief Handle death, it handles the death
 * @param LastHitInfo The last hit info
 */
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

/*
 * @brief Resolve hit box window, it resolves the hit box window
 * @param WindowId The window id
 * @param OutHitBoxNames The out hit box names
 * @param OutParams The out params
 * @return True if the hit box window is resolved, false otherwise
 */
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

    // Ĭ�ϴ��������� "HitWindow"
    static const FName DefaultWindowId(TEXT("HitWindow"));

    if (WindowId.IsNone() || WindowId == DefaultWindowId)
    {
        OutHitBoxNames = ActiveHitBoxNames;
        OutParams = ActiveHitBoxParams;
        return OutHitBoxNames.Num() > 0;
    }

    return false;
}

/*
 * @brief Enqueue action, it enqueues the action
 * @param Action The action
 */
void ABMPlayerCharacter::EnqueueAction(EBMCombatAction Action)
{
    ActionQueue.Reset();
    ActionQueue.Add(Action);
}

/*
 * @brief Consume next queued action, it consumes the next queued action
 * @param OutAction The out action
 * @return True if the next queued action is consumed, false otherwise
 */
bool ABMPlayerCharacter::ConsumeNextQueuedAction(EBMCombatAction& OutAction)
{
    if (ActionQueue.Num() <= 0) return false;
    OutAction = ActionQueue[0];
    ActionQueue.RemoveAt(0);
    return true;
}

/*
 * @brief Consume one queued normal attack, it consumes the one queued normal attack
 * @return True if the one queued normal attack is consumed, false otherwise
 */
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

/*
 * @brief Select skill spec, it selects the skill spec
 * @param Action The action
 * @param OutSpec The out spec
 * @param OutStaminaCost The out stamina cost
 * @return True if the skill spec is selected, false otherwise
 */
bool ABMPlayerCharacter::SelectSkillSpec(EBMCombatAction Action, FBMPlayerAttackSpec& OutSpec, float& OutStaminaCost) const
{
    for (const FBMPlayerSkillSlot& S : SkillSlots)
    {
        if (S.Action == Action)
        {
            OutSpec = S.Spec;
            OutStaminaCost = S.StaminaCost;
            return (OutSpec.Anim != nullptr);
        }
    }
    return false;
}

/*
 * @brief Get combo step, it gets the combo step
 * @param Index The index
 * @param Out The out
 * @return True if the combo step is got, false otherwise
 */
bool ABMPlayerCharacter::GetComboStep(int32 Index, FBMPlayerComboStep& Out) const
{
    if (!NormalComboSteps.IsValidIndex(Index)) return false;
    Out = NormalComboSteps[Index];
    return Out.Anim != nullptr;
}

/*
 * @brief Set active attack context, it sets the active attack context
 * @param HitBoxNames The hit box names
 * @param Params The params
 * @param bUninterruptible The uninterruptible
 * @param InterruptChance The interrupt chance
 * @param InterruptChanceOnHeavyHit The interrupt chance on heavy hit
 */
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

/*
 * @brief Clear active attack context, it clears the active attack context
 */
void ABMPlayerCharacter::ClearActiveAttackContext()
{
    bHasActiveAttackContext = false;
    ActiveHitBoxNames.Reset();
    ActiveHitBoxParams = FBMHitBoxActivationParams();

    bActiveUninterruptible = false;
    ActiveInterruptChance = 0.f;
    ActiveInterruptChanceOnHeavyHit = 0.f;
}

