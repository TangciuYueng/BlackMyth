// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Save/BMSaveData.h"

UBMSaveData::UBMSaveData()
{
    SaveSlotName = TEXT("DefaultSlot");
    Timestamp = FDateTime::Now();
    PlayerHP = 100.0f;
    PlayerLevel = 1;
    CurrentXP = 0.0f;
    SkillPoints = 0;
    Location = FVector::ZeroVector;
}