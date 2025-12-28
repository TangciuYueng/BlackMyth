#include "System/BMEnemyManagerSubsystem.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Character/Enemy/BMEnemyBoss.h"
#include "Character/Components/BMStatsComponent.h"
#include "System/Save/BMSaveGameSubsystem.h"
#include "System/Event/BMEventBusSubsystem.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

/*
 * @brief Initialize, it initializes the enemy manager subsystem
 * @param Collection The collection
 */
void UBMEnemyManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RegisteredEnemies.Empty();
    bLevelTransitionTriggered = false;

    // �����Զ�������ʱ��
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CleanupTimerHandle,
            this,
            &UBMEnemyManagerSubsystem::CleanupInvalidEnemies,
            CleanupInterval,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Initialized"));
}

/*
 * @brief Deinitialize, it deinitializes the enemy manager subsystem
 */
void UBMEnemyManagerSubsystem::Deinitialize()
{
    // �������ж���
    for (TWeakObjectPtr<ABMEnemyBase> Enemy : RegisteredEnemies)
    {
        if (ABMEnemyBase* EnemyPtr = Enemy.Get())
        {
            EnemyPtr->OnCharacterDied.RemoveAll(this);
        }
    }

    RegisteredEnemies.Empty();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CleanupTimerHandle);
        World->GetTimerManager().ClearTimer(LevelTransitionTimerHandle);
    }

    UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Deinitialized"));

    Super::Deinitialize();
}

/*
 * @brief Register enemy, it registers the enemy
 * @param Enemy The enemy
 */
void UBMEnemyManagerSubsystem::RegisterEnemy(ABMEnemyBase* Enemy)
{
    if (!Enemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] RegisterEnemy: Invalid enemy"));
        return;
    }

    // �����ظ�ע��
    for (const TWeakObjectPtr<ABMEnemyBase>& Existing : RegisteredEnemies)
    {
        if (Existing.Get() == Enemy)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] RegisterEnemy: %s already registered"), *Enemy->GetName());
            return;
        }
    }

    RegisteredEnemies.Add(Enemy);

    // ���������¼�
    Enemy->OnCharacterDied.AddUObject(this, &UBMEnemyManagerSubsystem::HandleEnemyDeath);

    UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Registered enemy: %s (Total: %d, Alive: %d)"),
        *Enemy->GetName(), GetTotalEnemyCount(), GetAliveEnemyCount());

    BroadcastCountChanged();
}

/*
 * @brief Unregister enemy, it unregisters the enemy
 * @param Enemy The enemy
 */
void UBMEnemyManagerSubsystem::UnregisterEnemy(ABMEnemyBase* Enemy)
{
    if (!Enemy)
    {
        return;
    }

    // ȡ������
    Enemy->OnCharacterDied.RemoveAll(this);

    // ���б����Ƴ�
    RegisteredEnemies.RemoveAll([Enemy](const TWeakObjectPtr<ABMEnemyBase>& Ptr)
    {
        return Ptr.Get() == Enemy;
    });

    UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Unregistered enemy: %s (Total: %d, Alive: %d)"),
        *Enemy->GetName(), GetTotalEnemyCount(), GetAliveEnemyCount());

    BroadcastCountChanged();
}

/*
 * @brief Get the alive enemy count, it gets the alive enemy count
 * @return The alive enemy count
 */
int32 UBMEnemyManagerSubsystem::GetAliveEnemyCount() const
{
    int32 Count = 0;

    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (ABMEnemyBase* Enemy = EnemyPtr.Get())
        {
            // Boss ���⴦�����׶�ת���ڼ���Ϊ���
            if (ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(Enemy))
            {
                if (Boss->IsInPhaseTransition())
                {
                    Count++;
                    continue;
                }
            }

            if (UBMStatsComponent* Stats = Enemy->GetStats())
            {
                if (!Stats->IsDead())
                {
                    Count++;
                }
            }
        }
    }

    return Count;
}

/*
 * @brief Get the total enemy count, it gets the total enemy count
 * @return The total enemy count
 */
int32 UBMEnemyManagerSubsystem::GetTotalEnemyCount() const
{
    int32 Count = 0;

    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (EnemyPtr.IsValid())
        {
            Count++;
        }
    }

    return Count;
}

/*
 * @brief Get the alive enemies, it gets the alive enemies
 * @param OutEnemies The out enemies
 * @return The alive enemies count
 */
int32 UBMEnemyManagerSubsystem::GetAliveEnemies(TArray<ABMEnemyBase*>& OutEnemies) const
{
    OutEnemies.Empty();

    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (ABMEnemyBase* Enemy = EnemyPtr.Get())
        {
            // Boss ���⴦�����׶�ת���ڼ�Ҳ�����б�
            if (ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(Enemy))
            {
                if (Boss->IsInPhaseTransition())
                {
                    OutEnemies.Add(Enemy);
                    continue;
                }
            }

            if (UBMStatsComponent* Stats = Enemy->GetStats())
            {
                if (!Stats->IsDead())
                {
                    OutEnemies.Add(Enemy);
                }
            }
        }
    }

    return OutEnemies.Num();
}

/*
 * @brief Get all enemies, it gets all enemies
 * @param OutEnemies The out enemies
 * @return The all enemies count
 */
int32 UBMEnemyManagerSubsystem::GetAllEnemies(TArray<ABMEnemyBase*>& OutEnemies) const
{
    OutEnemies.Empty();

    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (ABMEnemyBase* Enemy = EnemyPtr.Get())
        {
            OutEnemies.Add(Enemy);
        }
    }

    return OutEnemies.Num();
}

/*
 * @brief Are all enemies dead, it are all enemies dead
 * @return True if all enemies are dead, false otherwise
 */
bool UBMEnemyManagerSubsystem::AreAllEnemiesDead() const
{
    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (ABMEnemyBase* Enemy = EnemyPtr.Get())
        {
            // Boss ���⴦�����׶�ת���ڼ���Ϊ���
            if (ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(Enemy))
            {
                if (Boss->IsInPhaseTransition())
                {
                    return false; // Boss ����ת���׶Σ���Ϊ���
                }
            }

            if (UBMStatsComponent* Stats = Enemy->GetStats())
            {
                if (!Stats->IsDead())
                {
                    return false; // ���ִ�����
                }
            }
        }
    }

    return RegisteredEnemies.Num() > 0; // ���б�Ϊ���򷵻� false
}

/*
 * @brief Cleanup invalid enemies, it cleans up the invalid enemies
 */
void UBMEnemyManagerSubsystem::CleanupInvalidEnemies()
{
    const int32 OldCount = RegisteredEnemies.Num();

    RegisteredEnemies.RemoveAll([](const TWeakObjectPtr<ABMEnemyBase>& Ptr)
    {
        return !Ptr.IsValid();
    });

    const int32 Removed = OldCount - RegisteredEnemies.Num();

    if (Removed > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Cleaned up %d invalid enemies (Remaining: %d)"),
            Removed, RegisteredEnemies.Num());

        BroadcastCountChanged();
    }
}

/*
 * @brief Handle enemy death, it handles the enemy death
 * @param Victim The victim
 * @param LastHitInfo The last hit info
 */
void UBMEnemyManagerSubsystem::HandleEnemyDeath(ABMCharacterBase* Victim, const FBMDamageInfo& LastHitInfo)
{
    if (ABMEnemyBase* Enemy = Cast<ABMEnemyBase>(Victim))
    {
        UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Enemy died: %s (Alive: %d/%d)"),
            *Enemy->GetName(), GetAliveEnemyCount(), GetTotalEnemyCount());

        BroadcastCountChanged();

        // ����Ƿ����е��˶�������
        if (AreAllEnemiesDead())
        {
            UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] === ALL ENEMIES DEFEATED ==="));
            
            // ���ؿ�����������л�
            CheckLevelCompletionAndTransition();
        }
    }
}

/*
 * @brief Check level completion and transition, it checks the level completion and transition
 */
void UBMEnemyManagerSubsystem::CheckLevelCompletionAndTransition()
{
    // ��ֹ�ظ�����
    if (bLevelTransitionTriggered)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // ��ȡ��ǰ�ؿ�����
    FString CurrentLevelName = World->GetName();
    
    UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] Current Level: %s"), *CurrentLevelName);

    // ����Ƿ��ǵ�һ�أ���Ȼ������
    if (CurrentLevelName.Contains(TEXT("Stylized_Nature_ExampleScene")))
    {
        bLevelTransitionTriggered = true;
        
        UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] First level completed! Transitioning to boss level in 5 seconds..."));
        
        // ͨ���¼����߷���ʤ��֪ͨ
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UBMEventBusSubsystem* EventBus = GameInstance->GetSubsystem<UBMEventBusSubsystem>())
            {
                FText VictoryMessage = FText::FromString(TEXT("Victory! All enemies defeated! Loading next level in 5 seconds..."));
                EventBus->EmitNotify(VictoryMessage);
            }
        }
        
        // �ӳ�5����л���Boss�ؿ�
        World->GetTimerManager().SetTimer(
            LevelTransitionTimerHandle,
            this,
            &UBMEnemyManagerSubsystem::TransitionToNextLevel,
            5.0f,
            false
        );
    }
}

/*
 * @brief Transition to next level, it transitions to next level
 */
void UBMEnemyManagerSubsystem::TransitionToNextLevel()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[BMEnemyManagerSubsystem] TransitionToNextLevel: Invalid World"));
        return;
    }

    // ===== �л�ǰ����������� =====
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        if (UBMSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<UBMSaveGameSubsystem>())
        {
            UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] Auto-saving before level transition..."));
            
            // ʹ���Զ��浵��λ����λ0��
            bool bSaveSuccess = SaveSystem->SaveGame(0);
            
            if (bSaveSuccess)
            {
                UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] Auto-save successful!"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[BMEnemyManagerSubsystem] Auto-save failed! Player data may be lost."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[BMEnemyManagerSubsystem] SaveGameSubsystem not found! Cannot save player data."));
        }
    }

    // ���ͼ���֪ͨ
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        if (UBMEventBusSubsystem* EventBus = GameInstance->GetSubsystem<UBMEventBusSubsystem>())
        {
            FText LoadingMessage = FText::FromString(TEXT("Loading Boss Level..."));
            EventBus->EmitNotify(LoadingMessage);
        }
    }

    // Boss�ؿ�·��
    const FString BossLevelPath = TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01");
    
    UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] Loading Boss Level: %s"), *BossLevelPath);
    
    // �л���Boss�ؿ�
    UGameplayStatics::OpenLevel(World, FName(*BossLevelPath));
}

/*
 * @brief Broadcast count changed, it broadcasts the count changed
 */
void UBMEnemyManagerSubsystem::BroadcastCountChanged()
{
    const int32 AliveCount = GetAliveEnemyCount();
    const int32 TotalCount = GetTotalEnemyCount();

    OnEnemyCountChanged.Broadcast(AliveCount, TotalCount);
}
