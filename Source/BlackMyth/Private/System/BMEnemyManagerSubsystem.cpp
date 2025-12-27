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

void UBMEnemyManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    RegisteredEnemies.Empty();
    bLevelTransitionTriggered = false;

    // 启动自动清理定时器
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

void UBMEnemyManagerSubsystem::Deinitialize()
{
    // 清理所有订阅
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

void UBMEnemyManagerSubsystem::RegisterEnemy(ABMEnemyBase* Enemy)
{
    if (!Enemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] RegisterEnemy: Invalid enemy"));
        return;
    }

    // 避免重复注册
    for (const TWeakObjectPtr<ABMEnemyBase>& Existing : RegisteredEnemies)
    {
        if (Existing.Get() == Enemy)
        {
            UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] RegisterEnemy: %s already registered"), *Enemy->GetName());
            return;
        }
    }

    RegisteredEnemies.Add(Enemy);

    // 订阅死亡事件
    Enemy->OnCharacterDied.AddUObject(this, &UBMEnemyManagerSubsystem::HandleEnemyDeath);

    UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Registered enemy: %s (Total: %d, Alive: %d)"),
        *Enemy->GetName(), GetTotalEnemyCount(), GetAliveEnemyCount());

    BroadcastCountChanged();
}

void UBMEnemyManagerSubsystem::UnregisterEnemy(ABMEnemyBase* Enemy)
{
    if (!Enemy)
    {
        return;
    }

    // 取消订阅
    Enemy->OnCharacterDied.RemoveAll(this);

    // 从列表中移除
    RegisteredEnemies.RemoveAll([Enemy](const TWeakObjectPtr<ABMEnemyBase>& Ptr)
    {
        return Ptr.Get() == Enemy;
    });

    UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Unregistered enemy: %s (Total: %d, Alive: %d)"),
        *Enemy->GetName(), GetTotalEnemyCount(), GetAliveEnemyCount());

    BroadcastCountChanged();
}

int32 UBMEnemyManagerSubsystem::GetAliveEnemyCount() const
{
    int32 Count = 0;

    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (ABMEnemyBase* Enemy = EnemyPtr.Get())
        {
            // Boss 特殊处理：阶段转换期间视为存活
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

int32 UBMEnemyManagerSubsystem::GetAliveEnemies(TArray<ABMEnemyBase*>& OutEnemies) const
{
    OutEnemies.Empty();

    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (ABMEnemyBase* Enemy = EnemyPtr.Get())
        {
            // Boss 特殊处理：阶段转换期间也加入列表
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

bool UBMEnemyManagerSubsystem::AreAllEnemiesDead() const
{
    for (const TWeakObjectPtr<ABMEnemyBase>& EnemyPtr : RegisteredEnemies)
    {
        if (ABMEnemyBase* Enemy = EnemyPtr.Get())
        {
            // Boss 特殊处理：阶段转换期间视为存活
            if (ABMEnemyBoss* Boss = Cast<ABMEnemyBoss>(Enemy))
            {
                if (Boss->IsInPhaseTransition())
                {
                    return false; // Boss 正在转换阶段，视为存活
                }
            }

            if (UBMStatsComponent* Stats = Enemy->GetStats())
            {
                if (!Stats->IsDead())
                {
                    return false; // 发现存活敌人
                }
            }
        }
    }

    return RegisteredEnemies.Num() > 0; // 若列表为空则返回 false
}

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

void UBMEnemyManagerSubsystem::HandleEnemyDeath(ABMCharacterBase* Victim, const FBMDamageInfo& LastHitInfo)
{
    if (ABMEnemyBase* Enemy = Cast<ABMEnemyBase>(Victim))
    {
        UE_LOG(LogTemp, Log, TEXT("[BMEnemyManagerSubsystem] Enemy died: %s (Alive: %d/%d)"),
            *Enemy->GetName(), GetAliveEnemyCount(), GetTotalEnemyCount());

        BroadcastCountChanged();

        // 检查是否所有敌人都已死亡
        if (AreAllEnemiesDead())
        {
            UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] === ALL ENEMIES DEFEATED ==="));
            
            // 检查关卡完成条件并切换
            CheckLevelCompletionAndTransition();
        }
    }
}

void UBMEnemyManagerSubsystem::CheckLevelCompletionAndTransition()
{
    // 防止重复触发
    if (bLevelTransitionTriggered)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // 获取当前关卡名称
    FString CurrentLevelName = World->GetName();
    
    UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] Current Level: %s"), *CurrentLevelName);

    // 检查是否是第一关（自然场景）
    if (CurrentLevelName.Contains(TEXT("Stylized_Nature_ExampleScene")))
    {
        bLevelTransitionTriggered = true;
        
        UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] First level completed! Transitioning to boss level in 5 seconds..."));
        
        // 通过事件总线发送胜利通知
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UBMEventBusSubsystem* EventBus = GameInstance->GetSubsystem<UBMEventBusSubsystem>())
            {
                FText VictoryMessage = FText::FromString(TEXT("Victory! All enemies defeated! Loading next level in 5 seconds..."));
                EventBus->EmitNotify(VictoryMessage);
            }
        }
        
        // 延迟5秒后切换到Boss关卡
        World->GetTimerManager().SetTimer(
            LevelTransitionTimerHandle,
            this,
            &UBMEnemyManagerSubsystem::TransitionToNextLevel,
            5.0f,
            false
        );
    }
}

void UBMEnemyManagerSubsystem::TransitionToNextLevel()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("[BMEnemyManagerSubsystem] TransitionToNextLevel: Invalid World"));
        return;
    }

    // ===== 切换前保存玩家数据 =====
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        if (UBMSaveGameSubsystem* SaveSystem = GameInstance->GetSubsystem<UBMSaveGameSubsystem>())
        {
            UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] Auto-saving before level transition..."));
            
            // 使用自动存档槽位（槽位0）
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

    // 发送加载通知
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        if (UBMEventBusSubsystem* EventBus = GameInstance->GetSubsystem<UBMEventBusSubsystem>())
        {
            FText LoadingMessage = FText::FromString(TEXT("Loading Boss Level..."));
            EventBus->EmitNotify(LoadingMessage);
        }
    }

    // Boss关卡路径
    const FString BossLevelPath = TEXT("/Game/Stylized_Spruce_Forest/Demo/Maps/STZD_Demo_01");
    
    UE_LOG(LogTemp, Warning, TEXT("[BMEnemyManagerSubsystem] Loading Boss Level: %s"), *BossLevelPath);
    
    // 切换到Boss关卡
    UGameplayStatics::OpenLevel(World, FName(*BossLevelPath));
}

void UBMEnemyManagerSubsystem::BroadcastCountChanged()
{
    const int32 AliveCount = GetAliveEnemyCount();
    const int32 TotalCount = GetTotalEnemyCount();

    OnEnemyCountChanged.Broadcast(AliveCount, TotalCount);
}
