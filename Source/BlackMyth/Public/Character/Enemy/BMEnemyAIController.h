// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BMEnemyAIController.generated.h"

class UAISenseConfig_Sight;
class UAIPerceptionComponent;

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
    Patrol      UMETA(DisplayName = "Ñ²Âß"),
    Combat      UMETA(DisplayName = "Õ½¶·"),
    Chase       UMETA(DisplayName = "×·»÷"),
    Attack      UMETA(DisplayName = "¹¥»÷"),
    Flee        UMETA(DisplayName = "ÌÓÅÜ"),
    Dodge       UMETA(DisplayName = "ÉÁ±Ü")
};

UCLASS()
class BLACKMYTH_API ABMEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABMEnemyAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
    EEnemyAIState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Target")
    TObjectPtr<AActor> TargetPlayer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
    float LostTargetTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float LostTargetThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float FleeHealthThreshold = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
    float PlayerHealthThreshold = 250.0f;

    UFUNCTION(BlueprintCallable, Category = "AI|DecisionTree")
    void RunDecisionTree();

    UFUNCTION(BlueprintCallable, Category = "AI|State")
    void SetAIState(EEnemyAIState NewState);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Condition")
    bool IsPlayerInSight();
    virtual bool IsPlayerInSight_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Condition")
    float GetEnemyHealth();
    virtual float GetEnemyHealth_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Condition")
    float GetPlayerHealth();
    virtual float GetPlayerHealth_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Condition")
    bool IsPlayerInAttackRange();
    virtual bool IsPlayerInAttackRange_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Condition")
    bool IsPlayerAttacking();
    virtual bool IsPlayerAttacking_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Action")
    void ExecutePatrol();
    virtual void ExecutePatrol_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Action")
    void ExecuteChase();
    virtual void ExecuteChase_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Action")
    void ExecuteAttack();
    virtual void ExecuteAttack_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Action")
    void ExecuteFlee();
    virtual void ExecuteFlee_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Action")
    void ExecuteDodge();
    virtual void ExecuteDodge_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Action")
    void OnEnterCombat();
    virtual void OnEnterCombat_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AI|Action")
    void OnExitCombat();
    virtual void OnExitCombat_Implementation();

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    bool bFirstTimeSpotPlayer = true;
    void ExecuteCurrentStateBehavior();
};
