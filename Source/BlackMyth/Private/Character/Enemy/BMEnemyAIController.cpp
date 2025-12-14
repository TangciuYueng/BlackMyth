
// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Enemy/BMEnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Actor.h"

// ================== 条件检查接口 ==================
bool ABMEnemyAIController::IsPlayerInSight_Implementation()
{
    // 【需接入】UAIPerceptionComponent感知结果或自定义视线检测
    // 返回true表示主角在视野内
    return false;
}

float ABMEnemyAIController::GetEnemyHealth_Implementation()
{
    // 【需接入】敌人角色的血量组件
    // 例如：Cast<ABMEnemyBase>(GetPawn())->GetHealth();
    return 1000.f;
}

float ABMEnemyAIController::GetPlayerHealth_Implementation()
{
    // 【需接入】主角的血量组件
    // 例如：TargetPlayer->GetHealth();
    return 1000.f;
}

bool ABMEnemyAIController::IsPlayerInAttackRange_Implementation()
{
    // 【需接入】主角与敌人距离检测
    // 例如：FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation()) < AttackRange
    return false;
}

bool ABMEnemyAIController::IsPlayerAttacking_Implementation()
{
    // 【需接入】主角攻击状态（动画或战斗组件）
    // 例如：TargetPlayer->IsAttacking();
    return false;
}

// ================== 行为执行接口 ==================
void ABMEnemyAIController::ExecutePatrol_Implementation()
{
    // 【需实现】巡逻行为
    // 例如：移动到巡逻点
}

void ABMEnemyAIController::ExecuteChase_Implementation()
{
    // 【需实现】追击主角行为
    // 例如：MoveToActor(TargetPlayer)
}

void ABMEnemyAIController::ExecuteAttack_Implementation()
{
    // 【需实现】攻击行为
    // 例如：调用敌人攻击函数
}

void ABMEnemyAIController::ExecuteFlee_Implementation()
{
    // 【需实现】逃跑行为
    // 例如：远离主角方向移动
}

void ABMEnemyAIController::ExecuteDodge_Implementation()
{
    // 【需实现】闪避行为
    // 例如：快速移动到远离主角的位置
}

void ABMEnemyAIController::OnEnterCombat_Implementation()
{
    // 【需实现】进入战斗状态时的初始化
}

void ABMEnemyAIController::OnExitCombat_Implementation()
{
    // 【需实现】退出战斗状态时的清理
}

void ABMEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // 【需实现】感知回调，更新TargetPlayer等
}
ABMEnemyAIController::ABMEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // 初始化感知组件
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (AIPerceptionComp && SightConfig)
    {
        AIPerceptionComp->ConfigureSense(*SightConfig);
        AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
    }
    CurrentState = EEnemyAIState::Patrol;
    LostTargetTimer = 0.f;
}

void ABMEnemyAIController::BeginPlay()
{
    Super::BeginPlay();
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABMEnemyAIController::OnPerceptionUpdated);
    }
    CurrentState = EEnemyAIState::Patrol;
    LostTargetTimer = 0.f;
    bFirstTimeSpotPlayer = true;
}

void ABMEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    RunDecisionTree();
}

void ABMEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    // 可在此处缓存敌人角色指针
}

void ABMEnemyAIController::RunDecisionTree()
{
    // 决策树主逻辑框架
    // 1. 检查主角是否在视野内
    if (IsPlayerInSight())
    {
        LostTargetTimer = 0.f;
        if (bFirstTimeSpotPlayer)
        {
            SetAIState(EEnemyAIState::Combat);
            bFirstTimeSpotPlayer = false;
            OnEnterCombat();
        }
        // 2. 判断逃跑条件
        if (GetEnemyHealth() < FleeHealthThreshold && GetPlayerHealth() > PlayerHealthThreshold)
        {
            SetAIState(EEnemyAIState::Flee);
        }
        // 3. 判断主角是否在攻击范围内
        else if (IsPlayerInAttackRange())
        {
            SetAIState(EEnemyAIState::Attack);
        }
        // 4. 判断主角是否正在攻击
        else if (IsPlayerAttacking())
        {
            SetAIState(EEnemyAIState::Dodge);
        }
        else
        {
            SetAIState(EEnemyAIState::Chase);
        }
    }
    else
    {
        // 主角不在视野内，计时
        LostTargetTimer += GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
        if (LostTargetTimer > LostTargetThreshold)
        {
            SetAIState(EEnemyAIState::Patrol);
            bFirstTimeSpotPlayer = true;
            OnExitCombat();
        }
    }
    ExecuteCurrentStateBehavior();
}

void ABMEnemyAIController::SetAIState(EEnemyAIState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
    }
}

void ABMEnemyAIController::ExecuteCurrentStateBehavior()
{
    switch (CurrentState)
    {
    case EEnemyAIState::Patrol:
        ExecutePatrol();
        break;
    case EEnemyAIState::Combat:
        // 战斗状态下可做初始化
        break;
    case EEnemyAIState::Chase:
        ExecuteChase();
        break;
    case EEnemyAIState::Attack:
        ExecuteAttack();
        break;
    case EEnemyAIState::Flee:
        ExecuteFlee();
        break;
    case EEnemyAIState::Dodge:
        ExecuteDodge();
        break;
    default:
        break;
    }
}

