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

USTRUCT()
struct FBMHitRecord
{
    GENERATED_BODY()

    int32 TotalHits = 0;                 // 当前窗口该目标总命中次数
    TMap<FName, int32> HitBoxHits;       // 当前窗口该目标按 HitBoxName 的命中次数
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

    /** 元素类型（物理/火焰等） */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Damage")
    EBMElementType ElementType = EBMElementType::Physical;

    /** 默认受击反馈类型 */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Damage")
    EBMHitReaction DefaultReaction = EBMHitReaction::Light;

    /**
     * 击退强度
     *
     * 用于构建 Knockback 向量：Dir * KnockbackStrength
     */
    UPROPERTY(EditAnywhere, Category = "BM|HitBox|Damage")
    float KnockbackStrength = 0.f;
};


/**
 * 攻击判定组件（HitBox）
 *
 * 负责：
 * 根据 FBMHitBoxDefinition 创建并维护多个 UBoxComponent 判定盒（HitBoxes）
 * 在指定攻击窗口内启用某个 HitBox（ActivateHitBox/DeactivateHitBox）
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
    // 新：按名字列表激活多个 HitBox（一次攻击窗口）
    void ActivateHitBoxesByNames(const TArray<FName>& HitBoxNames, const FBMHitBoxActivationParams& Params);

    // 新：关闭指定名字列表的 HitBox（窗口结束）
    void DeactivateHitBoxesByNames(const TArray<FName>& HitBoxNames);

    // 新：关闭全部 HitBox（保险用）
    void DeactivateAllHitBoxes();



    ///**
    // * 启用指定类型的 HitBox
    // *
    // * 行为：
    // * - 根据 Type 查找定义（FindDefByType），若不存在则回退 Default
    // * - 确保对应 UBoxComponent 已创建并附着到 Mesh（EnsureCreated）
    // * - 清空本次挥砍命中列表（HitActorsThisSwing）
    // * - 将 HitBox 碰撞启用为 QueryOnly
    // *
    // * @param Type 要启用的 HitBox 类型（轻击/重击/技能等）
    // */
    //void ActivateHitBox(EBMHitBoxType Type);

    ///**
    // * 禁用当前激活的 HitBox
    // *
    // * 行为：
    // * - 将 ActiveHitBox 的碰撞关闭为 NoCollision
    // * - 清空 ActiveHitBox/ActiveHitBoxName 并重置命中列表
    // *
    // * 该函数应在攻击窗口结束时调用
    // */
    //void DeactivateHitBox();

    /** 设置基础伤害覆盖值 */
    void SetDamage(float NewDamage) { Damage = NewDamage; }

    /**
     * 重置本次挥砍的命中列表
     *
     * 用于控制同一攻击窗口内只结算一次的行为
     */
    void ResetHitList();

    /**
     * 注册一个 HitBox 定义
     *
     * 该函数仅记录配置，不立即创建碰撞体；BeginPlay/ActivateHitBox 时才会 EnsureCreated
     * 在构造函数或 BeginPlay 之前调用
     *
     * @param Def HitBox 定义配置
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
     * 获取 Owner 的 SkeletalMeshComponent
     *
     * 约定 Owner 为 ACharacter 派生类，使用 ACharacter::GetMesh()
     * @return Owner 的 Mesh；若 Owner 非 ACharacter 或 Mesh 不存在则返回 nullptr
     */
    USkeletalMeshComponent* ResolveOwnerMesh() const;

    /**
     * 将 Owner 解析为 ABMCharacterBase
     *
     * 用于构造 FBMDamageInfo 的 InstigatorActor，并读取 Stats.Attack 等数据
     * @return Owner 的 ABMCharacterBase 指针；失败返回 nullptr
     */
    ABMCharacterBase* ResolveOwnerCharacter() const;

    /**
     * 将 HitBox 类型映射为默认的 FName Key
     *
     * @param Type HitBox 类型
     * @return 对应的默认名称
     */
    static FName TypeToName(EBMHitBoxType Type);

    /**
     * 确保指定定义对应的 UBoxComponent 已创建并完成初始化
     *
     * 初始化内容
     * - NewObject 创建并注册到 Owner
     * - 附着到 Mesh 的 Socket/Bone
     * - 设置形状与相对变换
     * - 设置碰撞为 NoCollision + Overlap
     * - 绑定 OnComponentBeginOverlap 到 OnHitBoxOverlap
     *
     * @param Def HitBox 配置定义
     */
    void EnsureCreated(const FBMHitBoxDefinition& Def);

    /**
     * 根据 HitBox 类型查找定义
     *
     * 返回第一个 Type 匹配的定义
     * 若不存在，则回退查找 Default
     *
     * @param Type HitBox 类型
     * @return 定义指针；若不存在任何可用定义则返回 nullptr
     */
    const FBMHitBoxDefinition* FindDefByType(EBMHitBoxType Type) const;

    void SetHitBoxCollisionEnabled(FName HitBoxName, bool bEnabled);
    TArray<FName> FindNamesByType(EBMHitBoxType Type) const;

    /**
     * HitBox Overlap 回调：命中处理入口
     *
     * 过滤规则：
     * - 仅在 ActiveHitBox 存在且启用时处理
     * - 忽略 Owner 自身与重复命中目标（HitActorsThisSwing）
     * - 要求 OtherActor 可转换为 ABMCharacterBase
     * - 要求 OtherComp 带有 "BM_HurtBox" Tag
     *
     * 命中后：
     * - 构造 FBMDamageInfo
     * - 调用 Victim->TakeDamageFromHit(Info) 执行最终结算
     *
     * @param OverlappedComponent 发生 Overlap 的 HitBox 组件
     * @param OtherActor 被命中的 Actor
     * @param OtherComp 被命中的组件（应为 HurtBox）
     * @param OtherBodyIndex 组件 Body Index（未使用）
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

    // 当前窗口激活的 HitBox 名字集合（支持多开）
    UPROPERTY(Transient)
    TSet<FName> ActiveHitBoxNames;

    // 组件 -> HitBoxName 的反查表（Overlap 时 O(1) 找到属于哪个定义）
    UPROPERTY(Transient)
    TMap<TObjectPtr<UPrimitiveComponent>, FName> ComponentToHitBoxName;

    // Name -> Definitions 下标，加速 Def 查找（避免每次 overlap 遍历）
    UPROPERTY(Transient)
    TMap<FName, int32> NameToDefIndex;
    
    // 当前攻击窗口参数
    FBMHitBoxActivationParams ActiveWindowParams;

    // 命中记录（去重/多段命中用）
    TMap<TWeakObjectPtr<AActor>, FBMHitRecord> HitRecordsThisWindow;
};
