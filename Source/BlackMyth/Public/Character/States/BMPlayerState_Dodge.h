// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Components/BMCharacterState.h"
#include "BMPlayerState_Dodge.generated.h"

class UCharacterMovementComponent;

UCLASS()
class BLACKMYTH_API UBMPlayerState_Dodge : public UBMCharacterState
{
	GENERATED_BODY()
public:
    virtual void OnEnter(float DeltaTime) override;
    virtual void OnExit(float DeltaTime) override;
    virtual bool CanTransitionTo(FName StateName) const override;

private:
    void FinishDodge();
    void StepDodge();
    bool HasWalkableFloorAt(const FVector& WorldPos, const class ACharacter* Char) const;
private:
    FTimerHandle TimerHandleFinish;
    FTimerHandle TimerHandleStep;

    bool bFinished = false;

    FVector LockedDir = FVector::ForwardVector;

    float TotalDistance = 0.f;
    float TraveledDistance = 0.f;
    float WindowDuration = 0.f;

    float LastStepTime = 0.f;

    // 防止飞出平台的探测参数
    float LedgeProbeUp = 50.f;
    float LedgeProbeDown = 150.f;

    // 保存/恢复移动参数
    bool bSavedOrientToMovement = true;
    float SavedMaxWalkSpeed = 0.f;
};
