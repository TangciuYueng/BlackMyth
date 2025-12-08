#include "ABMEnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h" // 用于调试绘制

AABMEnemyBase::AABMEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AABMEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // 可以在这里处理简单的逻辑，但主要 AI 逻辑交给 Controller
}

AActor* AABMEnemyBase::DetectPlayer()
{
    // 1. 获取玩家 (假设是单人游戏，取 Index 0)
    // 优化：实际项目中建议在 BeginPlay 时缓存，或通过 Subsystem 获取
    AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);
    
    if (!PlayerActor) return nullptr;

    // 2. 距离检查
    float DistSq = FVector::DistSquared(GetActorLocation(), PlayerActor->GetActorLocation());
    
    // 如果已经锁定了目标，我们使用较大的“脱战距离”来判断是否丢失
    // 如果还没锁定，使用较小的“警戒距离”
    float CheckRange = CachedPlayerTarget ? LoseAggroRange : AggroRange;
    
    if (DistSq > (CheckRange * CheckRange))
    {
        CachedPlayerTarget = nullptr;
        return nullptr;
    }

    // 3. 视线检查 (Line of Sight)
    // 防止穿墙看到玩家
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(PlayerActor); // 忽略自身和目标，只检测障碍物

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        GetActorLocation(), // 建议优化：用 GetMesh()->GetSocketLocation("Head")
        PlayerActor->GetActorLocation(),
        ECC_Visibility, // 使用可见性通道
        Params
    );

    // 如果中间有障碍物 (bHit == true)，则视为不可见
    // 注意：LineTrace 返回 true 表示撞到了东西（障碍物）
    // 但我们需要确认撞到的是不是世界静态物体。
    // 这里简单处理：如果 LineTrace 没撞到任何东西，说明视线通畅
    // 或者如果撞到了东西，但我们还是想确认是否真的被遮挡
    
    // 更严谨的逻辑：直接 Trace 到玩家，如果 Hit Actor == Player 则可见
    // 这里用反向逻辑：如果中间没有阻挡，或者阻挡非常远
    if (!bHit) 
    {
        CachedPlayerTarget = PlayerActor;
        
        // 调试绘制：看到玩家时画红线
        // DrawDebugLine(GetWorld(), GetActorLocation(), PlayerActor->GetActorLocation(), FColor::Red, false, 0.1f);
        
        return PlayerActor;
    }

    // 视线被遮挡
    CachedPlayerTarget = nullptr;
    return nullptr;
}

void AABMEnemyBase::DropLoot()
{
    if (LootTable.Num() == 0) return;

    for (const FBMLootItem& LootItem : LootTable)
    {
        float Roll = FMath::FRand(); // 0.0 - 1.0
        if (Roll <= LootItem.DropChance)
        {
            int32 Qty = FMath::RandRange(LootItem.MinQuantity, LootItem.MaxQuantity);
            UE_LOG(LogTemp, Log, TEXT("Enemy %s dropped %s x%d"), *GetName(), *LootItem.ItemId.ToString(), Qty);
            // 这里调用你的 InventorySubsystem 或生成 PickupActor
        }
    }
}

#include "Engine/AssetManager.h" // 用于加载软引用

void AABMEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    // 如果设置了 EnemyID，自动初始化
    if (!EnemyID.IsNone())
    {
        InitEnemyData();
    }
}

// TODO Deferred Spawn
void AABMEnemyBase::InitEnemyData()
{
    // 1. 获取 Subsystem
    UBMDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UBMDataSubsystem>();
    if (!DataSubsystem) return;

    // 2. 获取数据行
    const FBMEnemyData* DataRow = DataSubsystem->GetEnemyData(EnemyID);
    if (!DataRow)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find Enemy Data for ID: %s"), *EnemyID.ToString());
        return;
    }

    // 3. 应用数值属性 (Stats)
    // 假设 StatsComponent 是你的属性组件
    if (StatsComponent) 
    {
        StatsComponent->SetMaxHP(DataRow->MaxHP);
        StatsComponent->SetHP(DataRow->MaxHP);
        StatsComponent->SetBaseAttack(DataRow->AttackPower);
    }

    // 4. 加载并应用 行为树 (AI)
    // 注意：BehaviorTreePath 是软引用，需要 TryLoad() 转为硬引用
    if (!DataRow->BehaviorTreePath.IsNull())
    {
        UBehaviorTree* LoadedBT = Cast<UBehaviorTree>(DataRow->BehaviorTreePath.TryLoad());
        if (LoadedBT)
        {
            // 将加载的 BT 赋值给成员变量，供 AIController 读取
            BehaviorTreeAsset = LoadedBT;
            
            // 如果 AIController 已经 possess 了，可能需要重启逻辑
            AAIController* AIC = Cast<AAIController>(GetController());
            if (AIC)
            {
                // 具体的 RunBehaviorTree 逻辑通常在 Controller 里，或者这里手动推一下
                // AIC->RunBehaviorTree(BehaviorTreeAsset);
            }
        }
    }

    // 5. 加载并应用 网格体 (Skeletal Mesh)
    if (!DataRow->MeshPath.IsNull())
    {
        USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(DataRow->MeshPath.TryLoad());
        if (LoadedMesh)
        {
            GetMesh()->SetSkeletalMesh(LoadedMesh);
        }
    }

    // 6. 加载并应用 动画蓝图 (AnimBP)
    if (!DataRow->AnimBPPath.IsNull())
    {
        UClass* LoadedAnimBP = DataRow->AnimBPPath.TryLoadClass<UAnimInstance>();
        if (LoadedAnimBP)
        {
            GetMesh()->SetAnimInstanceClass(LoadedAnimBP);
        }
    }
}