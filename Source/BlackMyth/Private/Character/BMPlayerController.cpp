// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BMPlayerController.h"
#include "BMGameInstance.h"
#include "System/UI/BMUIManagerSubsystem.h"
#include "UI/BMMainWidget.h"
#include "UI/BMPauseMenuWidget.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Components/BMStatsComponent.h"
#include "Core/BMTypes.h"
#include "System/Event/BMEventBusSubsystem.h"

void ABMPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (UGameInstance* GI = GetGameInstance())
    {
        UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (UIManager)
        {
            // ����ָ�����˵��ؿ���ʾ���˵�
            const UWorld* World = GetWorld();
            // Compare against the asset name of the startup map. For full path "/Game/emptymap/emptymap" the asset name is "emptymap".
            const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(const_cast<UObject*>(static_cast<const UObject*>(World)), /*bRemovePrefixPIE*/ true);
            const FString ExpectedLevelName = TEXT("emptymap");
            if (CurrentLevelName != ExpectedLevelName)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: CurrentLevelName=%s, expected startup=%s"), *CurrentLevelName, *ExpectedLevelName);
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Skipping auto main menu on non-startup level: %s"), *CurrentLevelName);
                return;
            }
            TSubclassOf<UBMMainWidget> ClassToUse = nullptr;
            if (BMGI && BMGI->MainMenuClass.IsValid())
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Using GameInstance MainMenuClass."));
                ClassToUse = BMGI->MainMenuClass.Get();
            }
            if (!ClassToUse)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/UI/WBP_MainMenu.WBP_MainMenu_C"));
                ClassToUse = LoadClass<UBMMainWidget>(nullptr, TEXT("/Game/UI/WBP_MainMenu.WBP_MainMenu_C"));
            }
            //if (!ClassToUse)
            //{
             //   UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/BlackMyth/UI/WBP_MainMenu.WBP_MainMenu_C"));
             //   ClassToUse = LoadClass<UBMMainWidget>(nullptr, TEXT("/Game/BlackMyth/UI/WBP_MainMenu.WBP_MainMenu_C"));
           // }

            if (ClassToUse)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Showing MainMenu with class %s"), *ClassToUse->GetName());
                UIManager->ShowMainMenu(ClassToUse);

                // �л�Ϊ UI Only ģʽ����ʾ�����ȷ���˵��ɼ��Ϳɽ���
                FInputModeUIOnly InputMode;
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                SetInputMode(InputMode);
                bShowMouseCursor = true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ABMPlayerController: MainMenu class not found. Check path or GameInstance config."));
            }
        }
    }
}

void ABMPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        // ��ʹ�� C++ �󶨣������� Project Settings �� Action Mappings
        // ֱ�Ӱ� Tab ���Դ���ͣ�˵�
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ABMPlayerController::TogglePauseMenu);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound Tab to TogglePauseMenu via C++"));

        // [TEST] Bind K to apply 50% actual damage to the player
        InputComponent->BindKey(EKeys::K, IE_Pressed, this, &ABMPlayerController::ApplyHalfHPDamage);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound K to ApplyHalfHPDamage via C++"));

        // [TEST] Bind L to consume 25 stamina for testing stamina bar updates
        InputComponent->BindKey(EKeys::L, IE_Pressed, this, &ABMPlayerController::ConsumeStaminaTest);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound L to ConsumeStaminaTest via C++"));

        // Bind 1/2 to trigger skill cooldown timers
        InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ABMPlayerController::StartSkill1Cooldown);
        InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ABMPlayerController::StartSkill2Cooldown);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound 1/2 keys to trigger skill cooldown timers"));
    }
}

// [TEST] Apply 50% of current MaxHP as actual damage via BMStatsComponent
void ABMPlayerController::ApplyHalfHPDamage()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyHalfHPDamage: No pawn possessed."));
        return;
    }
    UBMStatsComponent* Stats = MyPawn->FindComponentByClass<UBMStatsComponent>();
    if (!Stats)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyHalfHPDamage: BMStatsComponent not found on pawn %s."), *MyPawn->GetName());
        return;
    }
    FBMStatBlock& Block = Stats->GetStatBlockMutable();
    const float DamageAmount = Block.MaxHP * 0.5f;
    FBMDamageInfo Info;
    Info.TargetActor = MyPawn;
    Info.InstigatorActor = this;
    Info.DamageValue = DamageAmount;
    Info.RawDamageValue = DamageAmount;
    Info.DamageType = EBMDamageType::TrueDamage; // ensure exact 50% HP taken, bypass Defense
    const float Applied = Stats->ApplyDamage(Info);
    UE_LOG(LogTemp, Log, TEXT("ApplyHalfHPDamage: Applied=%.2f, NewHP=%.2f/%.2f"), Applied, Block.HP, Block.MaxHP);
}

// [TEST] Consume 25 stamina to test stamina bar updates
void ABMPlayerController::ConsumeStaminaTest()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsumeStaminaTest: No pawn possessed."));
        return;
    }
    UBMStatsComponent* Stats = MyPawn->FindComponentByClass<UBMStatsComponent>();
    if (!Stats)
    {
        UE_LOG(LogTemp, Warning, TEXT("ConsumeStaminaTest: BMStatsComponent not found on pawn %s."), *MyPawn->GetName());
        return;
    }
    const float StaminaCost = 25.0f;
    const FBMStatBlock& Block = Stats->GetStatBlock();
    const float OldStamina = Block.Stamina;
    const bool bSuccess = Stats->TryConsumeStamina(StaminaCost);
    const float NewStamina = Stats->GetStatBlock().Stamina;
    UE_LOG(LogTemp, Log, TEXT("ConsumeStaminaTest: Consumed=%.2f, Success=%d, OldStamina=%.2f, NewStamina=%.2f/%.2f"), 
        StaminaCost, bSuccess, OldStamina, NewStamina, Block.MaxStamina);
}

void ABMPlayerController::StartSkill1Cooldown()
{
    TriggerSkillCooldown(TEXT("Skill1"), 10.f);
}

void ABMPlayerController::StartSkill2Cooldown()
{
    TriggerSkillCooldown(TEXT("Skill2"), 8.f);
}
void ABMPlayerController::TriggerSkillCooldown(FName SkillId, float TotalSeconds)
{
    UBMEventBusSubsystem* Bus = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBMEventBusSubsystem>() : nullptr;
    if (!Bus)
    {
        UE_LOG(LogTemp, Warning, TEXT("TriggerSkillCooldown: EventBus not found."));
        return;
    }

    if (SkillId == TEXT("Skill1"))
    {
        // Do not restart if cooldown already running
        if (GetWorldTimerManager().IsTimerActive(Skill1CooldownTimer))
        {
            UE_LOG(LogTemp, Log, TEXT("Skill1 cooldown already running (%fs remaining)."), Skill1Remaining);
            return;
        }
        Skill1Remaining = TotalSeconds;
        Bus->EmitSkillCooldown(SkillId, Skill1Remaining);
        GetWorldTimerManager().SetTimer(Skill1CooldownTimer, this, &ABMPlayerController::OnSkillCooldownTick_Skill1, 1.0f, true);
    }
    else if (SkillId == TEXT("Skill2"))
    {
        // Do not restart if cooldown already running
        if (GetWorldTimerManager().IsTimerActive(Skill2CooldownTimer))
        {
            UE_LOG(LogTemp, Log, TEXT("Skill2 cooldown already running (%fs remaining)."), Skill2Remaining);
            return;
        }
        Skill2Remaining = TotalSeconds;
        Bus->EmitSkillCooldown(SkillId, Skill2Remaining);
        GetWorldTimerManager().SetTimer(Skill2CooldownTimer, this, &ABMPlayerController::OnSkillCooldownTick_Skill2, 1.0f, true);
    }
}

void ABMPlayerController::OnSkillCooldownTick_Skill1()
{
    UBMEventBusSubsystem* Bus = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBMEventBusSubsystem>() : nullptr;
    if (!Bus)
    {
        GetWorldTimerManager().ClearTimer(Skill1CooldownTimer);
        return;
    }
    Skill1Remaining = FMath::Max(0.f, Skill1Remaining - 1.f);
    Bus->EmitSkillCooldown(TEXT("Skill1"), Skill1Remaining);
    if (Skill1Remaining <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(Skill1CooldownTimer);
    }
}

void ABMPlayerController::OnSkillCooldownTick_Skill2()
{
    UBMEventBusSubsystem* Bus = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBMEventBusSubsystem>() : nullptr;
    if (!Bus)
    {
        GetWorldTimerManager().ClearTimer(Skill2CooldownTimer);
        return;
    }
    Skill2Remaining = FMath::Max(0.f, Skill2Remaining - 1.f);
    Bus->EmitSkillCooldown(TEXT("Skill2"), Skill2Remaining);
    if (Skill2Remaining <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(Skill2CooldownTimer);
    }
}

void ABMPlayerController::TogglePauseMenu()
{
    UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: TogglePauseMenu invoked"));
    if (UGameInstance* GI = GetGameInstance())
    {
        UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (UIManager)
        {
            if (UIManager->IsPauseMenuVisible())
            {
                UIManager->HidePauseMenu();
                FInputModeGameOnly GameOnly;
                SetInputMode(GameOnly);
                bShowMouseCursor = false;
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Hide Pause -> GameOnly"));
                return;
            }

            TSubclassOf<UBMPauseMenuWidget> PauseClass = nullptr;
            if (BMGI && BMGI->PauseMenuClass.IsValid())
            {
                PauseClass = BMGI->PauseMenuClass.Get();
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Using GameInstance PauseMenuClass."));
            }
            if (!PauseClass)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
                PauseClass = LoadClass<UBMPauseMenuWidget>(nullptr, TEXT("/Game/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
            }
            if (!PauseClass)
            {
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/BlackMyth/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
                PauseClass = LoadClass<UBMPauseMenuWidget>(nullptr, TEXT("/Game/BlackMyth/UI/WBP_PauseMenu.WBP_PauseMenu_C"));
            }

            if (PauseClass)
            {
                UIManager->ShowPauseMenu(PauseClass);
                // �л�Ϊ UI Only ģʽ�Ա���ͣ�˵��ɽ���
                FInputModeUIOnly InputMode;
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                SetInputMode(InputMode);
                bShowMouseCursor = true;
                UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: TogglePauseMenu -> UIOnly + cursor"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ABMPlayerController: PauseMenu class not found. Check GI setting or asset path."));
            }
        }
    }
}

