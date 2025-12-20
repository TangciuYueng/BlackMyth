// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMEnemyState_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class BLACKMYTH_API UBMEnemyState_Dodge : public UBMCharacterState
{
	GENERATED_BODY()
public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    void StepDodge();
    bool HasWalkableFloorAt(const FVector& WorldPos, const class ACharacter* Char) const;
    void FinishDodge();
private:
    FTimerHandle TimerHandleFinish;
    FTimerHandle TimerHandleStep;

    bool bFinished = false;

    FVector LockedDir = FVector::BackwardVector;

    float TotalDistance = 0.f;
    float TraveledDistance = 0.f;
    float WindowDuration = 0.f;

    float LastStepTime = 0.f;

    float LedgeProbeUp = 50.f;
    float LedgeProbeDown = 160.f;
	
};
