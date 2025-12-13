// Fill out your copyright notice in the Description page of Project Settings.

#include "System/Save/BMSaveData.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

/**
 * 构造函数
 * 
 * 初始化存档数据
 */
UBMSaveData::UBMSaveData()
{
    SaveVersion = BM_SAVE_DATA_VERSION;
    SaveSlotName = TEXT("DefaultSlot");
    Timestamp = FDateTime::Now();
    
    // 初始化默认属性值
    MaxHP = 100.0f;
    HP = 100.0f;
    MaxMP = 0.0f;
    MP = 0.0f;
    MaxStamina = 100.0f;
    Stamina = 100.0f;
    Attack = 10.0f;
    Defense = 0.0f;
    MoveSpeed = 600.0f;
    
    PlayerLevel = 1;
    CurrentXP = 0.0f;
    SkillPoints = 0;
    
    Location = FVector::ZeroVector;
    Rotation = FRotator::ZeroRotator;
}

/**
 * 验证存档数据有效性
 * 
 * 检查版本号、基本数据、属性合理性等
 * 
 * @return 如果数据有效返回 true，否则返回 false
 */
bool UBMSaveData::IsValid() const
{
    // 检查版本号
    if (SaveVersion < 1 || SaveVersion > BM_SAVE_DATA_VERSION)
    {
        return false;
    }
    
    // 检查基本数据有效性
    if (SaveSlotName.IsEmpty())
    {
        return false;
    }
    
    // 检查属性合理性
    if (HP < 0.0f || MaxHP <= 0.0f || HP > MaxHP)
    {
        return false;
    }
    
    // 检查 MP 合理性
    if (MP < 0.0f || MaxMP < 0.0f || MP > MaxMP)
    {
        return false;
    }
    
    // 检查 Stamina 合理性
    if (Stamina < 0.0f || MaxStamina <= 0.0f || Stamina > MaxStamina)
    {
        return false;
    }
    
    if (PlayerLevel < 1)
    {
        return false;
    }
    
    return true;
}

/**
 * 获取存档元信息
 * 
 * 包含时间戳、位置等信息的元数据结构
 * 
 * @return 包含时间戳、位置等信息的元数据结构
 */
FBMSaveMeta UBMSaveData::GetSaveMeta() const
{
    FBMSaveMeta Meta;
    Meta.Timestamp = Timestamp;
    Meta.MapName = MapName;
    Meta.PlayerLocation = Location;
    Meta.PlayerRotation = Rotation;
    return Meta;
}