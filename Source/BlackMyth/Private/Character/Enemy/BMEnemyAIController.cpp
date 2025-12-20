#include "Character/Enemy/BMEnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Actor.h"

// ================== �������ӿ� ==================
bool ABMEnemyAIController::IsPlayerInSight_Implementation()
{
    // ������롿UAIPerceptionComponent��֪������Զ������߼��
    // ����true��ʾ��������Ұ��
    return false;
}

float ABMEnemyAIController::GetEnemyHealth_Implementation()
{
    // ������롿���˽�ɫ��Ѫ�����
    // ���磺Cast<ABMEnemyBase>(GetPawn())->GetHealth();
    return 1000.f;
}

float ABMEnemyAIController::GetPlayerHealth_Implementation()
{
    // ������롿���ǵ�Ѫ�����
    // ���磺TargetPlayer->GetHealth();
    return 1000.f;
}

bool ABMEnemyAIController::IsPlayerInAttackRange_Implementation()
{
    // ������롿��������˾�����
    // ���磺FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation()) < AttackRange
    return false;
}

bool ABMEnemyAIController::IsPlayerAttacking_Implementation()
{
    // ������롿���ǹ���״̬��������ս�������
    // ���磺TargetPlayer->IsAttacking();
    return false;
}

// ================== ��Ϊִ�нӿ� ==================
void ABMEnemyAIController::ExecutePatrol_Implementation()
{
    // ����ʵ�֡�Ѳ����Ϊ
    // ���磺�ƶ���Ѳ�ߵ�
}

void ABMEnemyAIController::ExecuteChase_Implementation()
{
    // ����ʵ�֡�׷��������Ϊ
    // ���磺MoveToActor(TargetPlayer)
}

void ABMEnemyAIController::ExecuteAttack_Implementation()
{
    // ����ʵ�֡�������Ϊ
    // ���磺���õ��˹�������
}

void ABMEnemyAIController::ExecuteFlee_Implementation()
{
    // ����ʵ�֡�������Ϊ
    // ���磺Զ�����Ƿ����ƶ�
}

void ABMEnemyAIController::ExecuteDodge_Implementation()
{
    // ����ʵ�֡�������Ϊ
    // ���磺�����ƶ���Զ�����ǵ�λ��
}

void ABMEnemyAIController::OnEnterCombat_Implementation()
{
    // ����ʵ�֡�����ս��״̬ʱ�ĳ�ʼ��
}

void ABMEnemyAIController::OnExitCombat_Implementation()
{
    // ����ʵ�֡��˳�ս��״̬ʱ������
}

void ABMEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // ����ʵ�֡���֪�ص�������TargetPlayer��
}
ABMEnemyAIController::ABMEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // ��ʼ����֪���
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
    // ���ڴ˴�������˽�ɫָ��
}

void ABMEnemyAIController::RunDecisionTree()
{
    // ���������߼����
    // 1. ��������Ƿ�����Ұ��
    if (IsPlayerInSight())
    {
        LostTargetTimer = 0.f;
        if (bFirstTimeSpotPlayer)
        {
            SetAIState(EEnemyAIState::Combat);
            bFirstTimeSpotPlayer = false;
            OnEnterCombat();
        }
        // 2. �ж���������
        if (GetEnemyHealth() < FleeHealthThreshold && GetPlayerHealth() > PlayerHealthThreshold)
        {
            SetAIState(EEnemyAIState::Flee);
        }
        // 3. �ж������Ƿ��ڹ�����Χ��
        else if (IsPlayerInAttackRange())
        {
            SetAIState(EEnemyAIState::Attack);
        }
        // 4. �ж������Ƿ����ڹ���
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
        // ���ǲ�����Ұ�ڣ���ʱ
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
        // ս��״̬�¿�����ʼ��
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

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ABMEnemyAIController::ABMEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
}

void ABMEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void ABMEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // ��ѡ��ͬ���ڰ壨������Ϊ���ߣ���������� BT Ҳ����
}

bool ABMEnemyAIController::RequestMoveToActor(AActor* Target, float AcceptanceRadius)
{
    if (!Target) return false;
    MoveToActor(Target, AcceptanceRadius, true, true, true, nullptr, true);
    return true;
}

bool ABMEnemyAIController::RequestMoveToLocation(const FVector& Location, float AcceptanceRadius)
{
    MoveToLocation(Location, AcceptanceRadius, true, true, false, true);
    return true;
}

void ABMEnemyAIController::RequestStopMovement()
{
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

bool ABMEnemyAIController::IsMoveActive() const
{
    const UPathFollowingComponent* P = GetPathFollowingComponent();
    if (!P) return false;
    return P->GetStatus() == EPathFollowingStatus::Moving;
}