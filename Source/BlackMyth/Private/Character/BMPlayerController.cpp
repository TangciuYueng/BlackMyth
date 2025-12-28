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
#include "Character/Components/BMExperienceComponent.h"
#include "UI/BMNotificationWidget.h"
#include "Character/Enemy/BMEnemyBoss.h"
#include "BlackMythCharacter.h"
#include "UI/UBMBookWidget.h"
#include "Engine/LocalPlayer.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/ConfigCacheIni.h"
#include "System/Save/BMSaveGlobals.h"

/*
 * @brief Constructor of the ABMPlayerController class
 */
ABMPlayerController::ABMPlayerController()
{
    static ConstructorHelpers::FClassFinder<UBMBookWidget> BookWidgetBP(TEXT("/Game/UI/WBP_Book.WBP_Book_C"));
    if (BookWidgetBP.Succeeded())
    {
        BookWidgetClass = BookWidgetBP.Class;
    }
}

/*
 * @brief Begin play, it begins the play
 */
void ABMPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Load persistent flag: whether intro book has been shown before, it loads the persistent flag
    bool bPersisted = false;
    if (GConfig->GetBool(TEXT("BlackMyth"), TEXT("HasShownIntroBook"), bPersisted, GGameIni))
    {
        bHasShownIntroBook = bPersisted;
    }

    // Bind to real intro video finished event from GameInstance
    if (UBMGameInstance* GI = Cast<UBMGameInstance>(GetGameInstance()))
    {
        GI->OnIntroVideoFinishedNative.AddUObject(this, &ABMPlayerController::OnIntroVideoFinished);
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        // Restore persistent player data (coins/exp/items) after level reload
        if (auto* BMGI = Cast<UBMGameInstance>(GI))
        {
            BMGI->RestorePlayerPersistentData(this);
        }

        // Ensure game is unpaused and input is in GameOnly after reload
        UGameplayStatics::SetGamePaused(this, false);
        FInputModeGameOnly GameOnly;
        SetInputMode(GameOnly);
        bShowMouseCursor = false;
        UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (UIManager)
        {
            // ����ָ�����˵��ؿ���ʾ���˵�
            const UWorld* World = GetWorld();
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

                // �l�Ϊ UI Only ģʽ����ʾ�����ȷ���˵��ɼ��Ϳɽ���
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

/*
 * @brief Setup input component, it setups the input component
 */
void ABMPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (InputComponent)
    {
        // ��ʹ�� C++ �󶨣������� Project Settings �� Action Mappings
        // ֱ�Ӱ� Tab ���Դ���ͣ�˵�
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ABMPlayerController::TogglePauseMenu);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound Tab to TogglePauseMenu via C++"));

        InputComponent->BindKey(EKeys::K, IE_Pressed, this, &ABMPlayerController::ApplyHalfHPDamage);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound K to ApplyHalfHPDamage via C++"));

        // Bind Skill1 to RightMouseButton, Skill2 to Q, Skill3 to E
        //InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ABMPlayerController::StartSkill1Cooldown);
        //InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ABMPlayerController::StartSkill2Cooldown);
        //InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ABMPlayerController::StartSkill3Cooldown);
        // Debug: L to add one level worth of XP
        InputComponent->BindKey(EKeys::L, IE_Pressed, this, &ABMPlayerController::DebugGainOneLevel);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound RMB->Skill1, Q->Skill2, E->Skill3, L->GainOneLevel"));

        // Debug: N to show a test notification
        InputComponent->BindKey(EKeys::N, IE_Pressed, this, &ABMPlayerController::DebugShowNotification);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound N to DebugShowNotification"));
        InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ABMPlayerController::Input_EnterPressed);
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Bound RMB->Skill1, Q->Skill2, E->Skill3, L->GainOneLevel, Enter->EndVideo"));
    }
}

/*
 * @brief Apply half HP damage, it applies the half HP damage
 */
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

/*
 * @brief Debug gain one level, it debugs the gain one level
 */
void ABMPlayerController::DebugGainOneLevel()
{
    UE_LOG(LogTemp, Log, TEXT("DebugGainOneLevel: invoked"));
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugGainOneLevel: No pawn."));
        return;
    }
    if (UBMExperienceComponent* XP = MyPawn->FindComponentByClass<UBMExperienceComponent>())
    {
        const int32 CurrentLevel = XP->GetLevel();
        const float Threshold = XP->GetMaxXPForNextLevel();
        const float Current = XP->GetCurrentXP();
        const float Delta = FMath::Max(0.f, Threshold - Current);
        XP->AddXP(Delta);
        UE_LOG(LogTemp, Log, TEXT("DebugGainOneLevel: Added %f XP for level %d->%d (current %f / need %f)"), Delta, CurrentLevel, CurrentLevel+1, Current, Threshold);
    }
}

/*
 * @brief Input enter pressed, it inputs the enter pressed
 */
void ABMPlayerController::Input_EnterPressed()
{
    UE_LOG(LogTemp, Log, TEXT("Input_EnterPressed: Key pressed. Checking for dead Phase 2 boss..."));

    if (UBMGameInstance* GI = Cast<UBMGameInstance>(GetGameInstance()))
    {
        if (GI->bIsBossPhase2Defeated)
        {
            if (GI->bHasWatchedEndVideo)
            {
                UE_LOG(LogTemp, Log, TEXT("Input_EnterPressed: EndVideo watched. Returning to Main Menu (emptymap)."));
                GI->StopLevelMusic();
                UGameplayStatics::OpenLevel(this, TEXT("emptymap"));
            }
            else if (GI->bIsEndMoviePlaying)
            {
                UE_LOG(LogTemp, Log, TEXT("Input_EnterPressed: EndVideo is playing. Stopping and returning to Main Menu."));
                GI->StopEndVideo();
                GI->StopLevelMusic();
                UGameplayStatics::OpenLevel(this, TEXT("emptymap"));
            }
            else
            {
                UE_LOG(LogTemp, Log, TEXT("Input_EnterPressed: Boss Phase 2 Defeated flag is set. Playing EndVideo."));
                // Use RequestPlayEndVideo to respect manual trigger configuration in GameInstance
                GI->RequestPlayEndVideo();
            }
            return;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Input_EnterPressed: Boss Phase 2 Defeated flag is NOT set."));
}

/*
 * @brief Consume stamina test, it consumes the stamina test
 */
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

/*
 * @brief Start skill 1 cooldown, it starts the skill 1 cooldown
 */
void ABMPlayerController::StartSkill1Cooldown()
{
    TriggerSkillCooldown(TEXT("Skill1"), 10.f);
}

/*
 * @brief Start skill 2 cooldown, it starts the skill 2 cooldown
 */
void ABMPlayerController::StartSkill2Cooldown()
{
    TriggerSkillCooldown(TEXT("Skill2"), 8.f);
}

/*
 * @brief Start skill 3 cooldown, it starts the skill 3 cooldown
 */
void ABMPlayerController::StartSkill3Cooldown()
{
    TriggerSkillCooldown(TEXT("Skill3"), 12.f);
}

/*
 * @brief Trigger skill cooldown, it triggers the skill cooldown
 * @param SkillId The skill id
 * @param TotalSeconds The total seconds
 */
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
    else if (SkillId == TEXT("Skill3"))
    {
        if (GetWorldTimerManager().IsTimerActive(Skill3CooldownTimer))
        {
            UE_LOG(LogTemp, Log, TEXT("Skill3 cooldown already running (%fs remaining)."), Skill3Remaining);
            return;
        }
        Skill3Remaining = TotalSeconds;
        Bus->EmitSkillCooldown(SkillId, Skill3Remaining);
        GetWorldTimerManager().SetTimer(Skill3CooldownTimer, this, &ABMPlayerController::OnSkillCooldownTick_Skill3, 1.0f, true);
    }
}

/*
 * @brief On skill cooldown tick skill 1, it on the skill cooldown tick skill 1
 */
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

/*
 * @brief On skill cooldown tick skill 2, it on the skill cooldown tick skill 2
 */
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

/*
 * @brief On skill cooldown tick skill 3, it on the skill cooldown tick skill 3
 */
void ABMPlayerController::OnSkillCooldownTick_Skill3()
{
    UBMEventBusSubsystem* Bus = GetGameInstance() ? GetGameInstance()->GetSubsystem<UBMEventBusSubsystem>() : nullptr;
    if (!Bus)
    {
        GetWorldTimerManager().ClearTimer(Skill3CooldownTimer);
        return;
    }
    Skill3Remaining = FMath::Max(0.f, Skill3Remaining - 1.f);
    Bus->EmitSkillCooldown(TEXT("Skill3"), Skill3Remaining);
    if (Skill3Remaining <= 0.f)
    {
        GetWorldTimerManager().ClearTimer(Skill3CooldownTimer);
    }
}

/*
 * @brief Toggle pause menu, it toggles the pause menu
 */
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
                // �l�Ϊ UI Only ģʽ�Ա���ͣ�˵��ɽ���
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

/*
 * @brief Debug show notification, it debugs the show notification
 */
void ABMPlayerController::DebugShowNotification()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugShowNotification: No GameInstance."));
        return;
    }

    UBMUIManagerSubsystem* UIManager = GI->GetSubsystem<UBMUIManagerSubsystem>();
    if (!UIManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("DebugShowNotification: UIManager subsystem not found."));
        return;
    }

    // Ensure the notification widget is on screen. Mimic main menu pattern: prefer GI NotificationClass, then fallback paths.
    if (!UIManager->IsNotificationVisible())
    {
        TSubclassOf<UBMNotificationWidget> NotificationClass = nullptr;
        UBMGameInstance* BMGI = Cast<UBMGameInstance>(GI);
        if (BMGI && BMGI->NotificationClass.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Using GameInstance NotificationClass."));
            NotificationClass = BMGI->NotificationClass.Get();
        }
        if (!NotificationClass)
        {
            UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/UI/WBP_Notification.WBP_Notification_C"));
            NotificationClass = LoadClass<UBMNotificationWidget>(nullptr, TEXT("/Game/UI/WBP_Notification.WBP_Notification_C"));
        }
        if (!NotificationClass)
        {
            UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Trying /Game/BlackMyth/UI/WBP_Notification.WBP_Notification_C"));
            NotificationClass = LoadClass<UBMNotificationWidget>(nullptr, TEXT("/Game/BlackMyth/UI/WBP_Notification.WBP_Notification_C"));
        }

        if (NotificationClass)
        {
            UE_LOG(LogTemp, Log, TEXT("ABMPlayerController: Showing Notification with class %s"), *NotificationClass->GetName());
            UIManager->ShowNotification(NotificationClass);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ABMPlayerController: Notification class not found. Check path or GameInstance config."));
        }
    }

    // Push the message via EventBus so the blueprint widget receives it
    UIManager->PushNotificationMessage(FText::FromString(MessageTest));
    UE_LOG(LogTemp, Log, TEXT("DebugShowNotification -> %s"), *MessageTest);
}

/*
 * @brief On intro video finished, it on the intro video finished
 */
void ABMPlayerController::OnIntroVideoFinished()
{
    // If we've already shown the book, skip
    if (bHasShownIntroBook)
    {
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController::OnIntroVideoFinished - book already shown, skipping."));
        return;
    }

    // If loading from a save, suppress the intro/book UI
    // suppression handled via BMSave global; if set, skip showing
    if (BMSave::bIsLoadingFromSave)
    {
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController::OnIntroVideoFinished - loading from save, suppressing book UI."));
        return;
    }

    ShowBookUI();
    bHasShownIntroBook = true;
    // Persist flag to game ini so the book won't show on subsequent runs
    GConfig->SetBool(TEXT("BlackMyth"), TEXT("HasShownIntroBook"), true, GGameIni);
    GConfig->Flush(false, GGameIni);
}

/*
 * @brief End play, it ends the play
 * @param EndPlayReason The end play reason
 */
void ABMPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UBMGameInstance* GI = Cast<UBMGameInstance>(GetGameInstance()))
    {
        GI->OnIntroVideoFinishedNative.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

/*
 * @brief Show book UI, it shows the book UI
 */
void ABMPlayerController::ShowBookUI()
{
    // Do not show book UI on level 2 (demo level)
    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), /*bRemovePrefixPIE=*/ true);
    const FString DisabledLevelName = TEXT("STZD_Demo_01");
    if (CurrentLevelName.Equals(DisabledLevelName, ESearchCase::IgnoreCase))
    {
        UE_LOG(LogTemp, Log, TEXT("ABMPlayerController::ShowBookUI - skipping book UI on level '%s'"), *CurrentLevelName);
        return;
    }

    if (!BookWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ABMPlayerController::ShowBookUI - BookWidgetClass is null. Check path or constructor initialization."));
        return;
    }

    if (BookWidgetClass)
    {
        if (!BookWidgetInstance)
        {
            BookWidgetInstance = CreateWidget<UBMBookWidget>(this, BookWidgetClass);
        }

        if (BookWidgetInstance && !BookWidgetInstance->IsInViewport())
        {
            BookWidgetInstance->AddToViewport(100); // High Z-order
            
            // Set input mode to UI only or GameAndUI to allow interaction
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(BookWidgetInstance->TakeWidget());
            SetInputMode(InputMode);
            bShowMouseCursor = true;
            BookWidgetInstance->SetKeyboardFocus();
        }
    }
}
