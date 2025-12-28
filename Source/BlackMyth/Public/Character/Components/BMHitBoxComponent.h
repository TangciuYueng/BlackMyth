#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "DrawDebugHelpers.h"
#include "BMHitBoxComponent.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class USkeletalMeshComponent;
class ABMCharacterBase;

/**
 * HitBox 系统日志分类
 *
 * 用于输出 HitBox 创建、启用/禁用、Overlap 命中与伤害构建等调试信息
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBMHitBox, Log, All);

/**
 * 命中记录结构
 *
 * 用于跟踪当前攻击窗口内对同一目标的命中次数，实现去重机制
 */
USTRUCT()
struct FBMHitRecord
{
    GENERATED_BODY()

    /** 当前窗口该目标总命中次数 */
    int32 TotalHits = 0;

    /** 当前窗口该目标按 HitBoxName 的命中次数 */
    TMap<FName, int32> HitBoxHits;
};


/**
 * HitBox 配置定义
 *
 * 描述一个可被运行时创建并挂接到骨骼 Socket/Bone 的攻击判定盒
 * 该定义用于
 * - HitBox 的命名与查找（Name 对应 HitBoxes Map Key）
 * - 挂接点（AttachSocketOrBone）与形状（BoxExtent/RelativeTransform）
 * - 伤害构建参数
 *
 * 该结构应在 BeginPlay 前完成注册（RegisterDefinition），以保证组件启动后即可创建碰撞体
 */
USTRUCT(BlueprintType)
struct FBMHitBoxDefinition
{
    GENERATED_BODY()

    /** HitBox 唯一标识名 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    FName Name = NAME_None;

    /** HitBox 逻辑类型 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    EBMHitBoxType Type = EBMHitBoxType::Default;

    /**
     * 挂接点 Socket/Bone 名
     *
     * HitBox 将附着到 Owner 的 SkeletalMeshComponent的该骨骼/插槽。
     * 若为空则附着到 Mesh 根
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    FName AttachSocketOrBone = NAME_None;

    /** HitBox 盒体半尺寸 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    FVector BoxExtent = FVector(8.f, 8.f, 8.f);

    /**
     * 相对挂接点的局部变换（位置/旋转/缩放）
     *
     * 用于对齐武器轨迹或手部动作
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    FTransform RelativeTransform;


    /** 额外伤害加成 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Damage")
    float AdditiveDamage = 0.f; 

    /** 对攻击者基础攻击的缩放系数（DamageValue = BaseAttack * DamageScale + AdditiveDamage） */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Damage")
    float DamageScale = 1.f;    

    /** 伤害类型（近战/远程/技能等） */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Damage")
    EBMDamageType DamageType = EBMDamageType::Melee;

    /** 默认受击反馈类型 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Damage")
    EBMHitReaction DefaultReaction = EBMHitReaction::Light;
};


/**
 * 攻击判定组件（HitBox）
 *
 * 负责：
 * 根据 FBMHitBoxDefinition 创建并维护多个 UBoxComponent 判定盒
 * 在指定攻击窗口内启用某个 HitBox
 * 处理 Overlap 命中，将命中事件转换为 FBMDamageInfo，并调用 Victim->TakeDamageFromHit 结算
 */
UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMHitBoxComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 初始化 Tick、默认配置以及运行时状态缓存。
     * 真实 HitBox 碰撞体在 BeginPlay 阶段创建
     */
    UBMHitBoxComponent();

    /**
     * 基础伤害覆盖值
     *
     * 若 > 0，则作为 BaseAttack 使用；否则从 Attacker 的 Stats.Attack 读取
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    float Damage = 0.f; 

    /** HitBox 默认元素类型 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox")
    EBMElementType ElementType = EBMElementType::Physical;


    /**
     * 运行时创建的 HitBox 碰撞体集合
     *
     * Key 为 FBMHitBoxDefinition::Name，Value 为对应的 UBoxComponent。
     */
    UPROPERTY(Transient)
    TMap<FName, TObjectPtr<UBoxComponent>> HitBoxes;

    /** 是否在 Tick 中绘制 HitBox 调试框（DrawDebugBox）仅用于开发调试 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Debug")
    bool bDebugDraw = false;

    /** 当前激活的 HitBox 绘制颜色 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Debug")
    FColor DebugColorActive = FColor::Red;        

    /** 未激活但已存在的 HitBox 绘制颜色 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Debug")
    FColor DebugColorInactive = FColor(180, 0, 0); 

    /** 调试框线宽 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Debug")
    float DebugLineThickness = 2.0f;

public:
    /**
     * 按名称列表激活多个 HitBox
     *
     * 在政击窗口开始时调用，根据参数启用指定的 HitBox 并设置去重策略
     *
     * @param HitBoxNames 要激活的 HitBox 名称列表
     * @param Params 激活参数
     */
    void ActivateHitBoxesByNames(const TArray<FName>& HitBoxNames, const FBMHitBoxActivationParams& Params);

    /**
     * 按名称列表关闭多个 HitBox
     *
     * 在攻击窗口结束时调用，禁用指定的 HitBox 碰撞检测
     *
     * @param HitBoxNames 要关闭的 HitBox 名称列表
     */
    void DeactivateHitBoxesByNames(const TArray<FName>& HitBoxNames);

    /**
     * 关闭所有 HitBox
     *
     * 立即禁用所有 HitBox 的碰撞检测并清空状态，用于强制重置或攻击被打断
     */
    void DeactivateAllHitBoxes();

    /**
     * 设置基础伤害覆盖值
     *
     * @param NewDamage 新的基础伤害值；若 > 0 则使用该值，否则从攻击者 Stats 读取
     */
    void SetDamage(float NewDamage) { Damage = NewDamage; }

    /**
     * 重置本次窗口的命中列表
     *
     * 清空当前攻击窗口的所有命中记录，允许相同目标再次被命中
     */
    void ResetHitList();

    /**
     * 注册 HitBox 定义
     *
     * @param Def HitBox 定义配置，包含名称、类型、尺寸、伤害参数等
     */
    void RegisterDefinition(const FBMHitBoxDefinition& Def);

protected:
    /**
     * 生命周期回调：组件开始运行
     *
     * 行为：
     * - 若未注册任何 Definitions，自动生成 Default 
     * - 遍历 Definitions，确保对应 UBoxComponent 已创建并注册 Overlap 回调
     * - 默认禁用 HitBox
     */
    virtual void BeginPlay() override;

    /**
     * 生命周期回调：组件结束运行
     *
     * 行为：
     * - 销毁运行时创建的 UBoxComponent
     * - 清理 Map、Active 指针与命中缓存
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * Tick：用于调试绘制 HitBox 框体
     *
     * 仅在 bDebugDraw 为 true 时执行 DrawDebugBox
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
/**
 * 获取 Owner 的骨骼网格组件
 *
 * @return Owner 的 SkeletalMeshComponent；若解析失败则返回 nullptr
 */
USkeletalMeshComponent* ResolveOwnerMesh() const;

/**
 * 将 Owner 解析为 ABMCharacterBase
 *
 * @return Owner 的 ABMCharacterBase 指针；解析失败返回 nullptr
 */
ABMCharacterBase* ResolveOwnerCharacter() const;

/**
 * 将 HitBox 类型映射为默认名称
 *
 * @param Type HitBox 类型
 * @return 对应的默认 FName
 */
static FName TypeToName(EBMHitBoxType Type);

/**
 * 确保指定定义对应的 UBoxComponent 已创建并完成初始化
 *
 * @param Def HitBox 配置定义
 */
void EnsureCreated(const FBMHitBoxDefinition& Def);

/**
 * 根据 HitBox 类型查找定义
 *
 * @param Type HitBox 类型
 * @return 定义指针；不存在则返回 nullptr
 */
const FBMHitBoxDefinition* FindDefByType(EBMHitBoxType Type) const;

/**
 * 设置 HitBox 碰撞启用状态
 *
 * @param HitBoxName HitBox 名称
 * @param bEnabled 是否启用碰撞
 */
void SetHitBoxCollisionEnabled(FName HitBoxName, bool bEnabled);

/**
 * 按类型查找所有匹配的 HitBox 名称
 *
 * @param Type HitBox 类型
 * @return 匹配的 HitBox 名称列表
 */
TArray<FName> FindNamesByType(EBMHitBoxType Type) const;

    /**
     * HitBox Overlap 回调：命中处理入口
     *
     * 过滤规则：
     * - 仅在 ActiveHitBox 存在且启用时处理
     * - 忽略 Owner 自身与重复命中目标
     * - 要求 OtherActor 可转换为 ABMCharacterBase
     * - 要求 OtherComp 带有 "BM_HurtBox" Tag
     *
     * 命中后：
     * - 构造 FBMDamageInfo
     * - 调用 Victim->TakeDamageFromHit(Info) 执行最终结算
     *
     * @param OverlappedComponent 发生 Overlap 的 HitBox 组件
     * @param OtherActor 被命中的 Actor
     * @param OtherComp 被命中的组件
     * @param OtherBodyIndex 组件 Body Index
     * @param bFromSweep 是否由 Sweep 导致
     * @param SweepResult Sweep 命中信息
     */
    UFUNCTION()
    void OnHitBoxOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

private:
    /**
     * HitBox 定义表。
     *
     * 每条定义描述一个 HitBox 的名称，绑定骨骼/尺寸/局部变换，伤害参数
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Definitions")
    TArray<FBMHitBoxDefinition> Definitions;

    // 当前窗口激活的 HitBox 名字集合
    UPROPERTY(Transient)
    TSet<FName> ActiveHitBoxNames;

    // 组件 -> HitBoxName 的反查表
    UPROPERTY(Transient)
    TMap<TObjectPtr<UPrimitiveComponent>, FName> ComponentToHitBoxName;

    // Name -> Definitions 下标
    UPROPERTY(Transient)
    TMap<FName, int32> NameToDefIndex;
    
    // 当前攻击窗口参数
    FBMHitBoxActivationParams ActiveWindowParams;

    // 命中记录
    TMap<TWeakObjectPtr<AActor>, FBMHitRecord> HitRecordsThisWindow;
};
