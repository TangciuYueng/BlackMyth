#include "Character/Enemy/States/BMEnemyState_Death.h"
#include "Character/Enemy/BMEnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "Character/Components/BMHitBoxComponent.h"

void UBMEnemyState_Death::OnEnter(float)
{
    ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext());
    if (!E) return;

    E->RequestStopMovement();

    if (UBMHitBoxComponent* HB = E->GetHitBox())
    {
        HB->DeactivateHitBox();
    }

    if (UCapsuleComponent* Cap = E->GetCapsuleComponent())
    {
        Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    const float Duration = E->PlayDeathOnce();
    const float Delay = (Duration > 0.f) ? (Duration + 0.2f) : 0.5f;

    FTimerDelegate D = FTimerDelegate::CreateWeakLambda(this, [this]() { FinishDeath(); });
    E->GetWorldTimerManager().SetTimer(DeathFinishHandle, D, Delay, false);
}

void UBMEnemyState_Death::FinishDeath()
{
    if (ABMEnemyBase* E = Cast<ABMEnemyBase>(GetContext()))
    {
        E->SetLifeSpan(0.1f);
    }
}
