#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/BMTypes.h"
#include "DrawDebugHelpers.h"



#include "BMHurtBoxComponent.generated.h"

class UBoxComponent;
class USkeletalMeshComponent;

/**
 * HurtBox 系统日志分类
 *
 * 用于输出 HurtBox 初始化、碰撞体创建、伤害修正与调试绘制等信息
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBMHurtBox, Log, All);

/**
 * HurtBox 受击判定组件
 *
 * 负责：
 * - 在 SkeletalMesh 的指定 Socket/Bone 上创建并维护一个 QueryOnly 的 UBoxComponent
 * - 作为 HitBox 命中过滤目标
 * - 在 Victim 侧受击结算前对 FBMDamageInfo 进行修正
 */
UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent))
class BLACKMYTH_API UBMHurtBoxComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 初始化默认倍率与调试参数，并配置 Tick
     */
    UBMHurtBoxComponent();

    /**
     * 部位伤害倍率
     *
     * - 头部/弱点：> 1
     */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox")
    float DamageMultiplier = 1.0f;

    /**
     * 弱点元素列表
     *
     * 当命中元素 InOutInfo.ElementType 命中该列表时，额外增伤
     * 具体倍率由 ModifyIncomingDamage 中策略决定
     */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox")
    TArray<EBMElementType> WeaknessTypes;

    /**
     * 抗性元素列表
     *
     * 当命中元素 InOutInfo.ElementType 命中该列表时，额外减伤
     * 具体倍率由 ModifyIncomingDamage 中策略决定
     */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox")
    TArray<EBMElementType> ResistanceTypes;


    /**
     * HurtBox 绑定到 SkeletalMesh 的 Socket/Bone 名称
     *
     * 创建的碰撞盒将挂接到该 Socket/Bone，并随动画与骨骼驱动移动
     */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox|Shape")
    FName AttachSocketOrBone = NAME_None;

    /**
     * 碰撞盒半径，单位为厘米
     *
     * 表示 UBoxComponent 的半尺寸
     */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox|Shape")
    FVector BoxExtent = FVector(12.f, 12.f, 12.f);

    /**
     * HurtBox 相对 Socket/Bone 的局部变换
     *
     * 用于微调碰撞盒的位置/旋转/缩放
     */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox|Shape")
    FTransform RelativeTransform;


    /**
     * 是否在 Tick 中绘制 HurtBox 的调试框
     */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox|Debug")
    bool bDebugDraw = false;

public:
    /**
     * 被命中后的通知回调
     *
     * 由 ABMCharacterBase 在 TakeDamageFromHit 结算完成并确认实际扣血后调用
     *
     * @param AppliedDamage 本次实际生效的伤害值
     */
    virtual void OnHit(float AppliedDamage);


    /**
     * 设置部位伤害倍率
     *
     * @param Multiplier 新倍率值
     */
    void SetDamageMultiplier(float Multiplier) { DamageMultiplier = Multiplier; }


    /**
     * 判断给定组件是否为该 HurtBox 所绑定的碰撞组件
     *
     * @param InComp 命中的组件指针
     * @return 若命中组件与本 HurtBox 绑定的碰撞组件一致则返回 true
     */
    bool IsBoundTo(const UPrimitiveComponent* InComp) const { return BoundComponent.Get() == InComp; }

    /**
     * 获取 HurtBox 所绑定的底层碰撞组件
     *
     * @return 绑定的 UPrimitiveComponent；若尚未创建则可能为 nullptr
     */
    UPrimitiveComponent* GetBoundComponent() const { return BoundComponent.Get(); }


    /**
     * 修正即将结算的伤害信息
     *
     * 包括：
     * - 按部位倍率缩放 DamageValue
     * - 按 Weakness/Resistance 对元素伤害进行增减
     * - 当 HitReaction 未指定时，给出推荐受击反馈
     *
     * @param InOutInfo 输入为命中构建的伤害信息，输出为修正后的伤害信息
     */
    virtual void ModifyIncomingDamage(FBMDamageInfo& InOutInfo) const;

    void SetHurtBoxEnabled(bool bEnabled);
    bool IsHurtBoxEnabled() const;

    /** Debug 绘制颜色 */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox|Debug")
    FColor DebugColor = FColor::Green;

    /** Debug 绘制线宽 */
    UPROPERTY(EditAnywhere, Category = "BM|HurtBox|Debug")
    float DebugLineThickness = 2.0f;

protected:
    /**
     * 生命周期回调：组件开始运行时调用
     *
     * - 创建 HurtBox 碰撞体
     * - 将碰撞体挂接到 SkeletalMesh 的指定 Socket/Bone
     */
    virtual void BeginPlay() override;

    /**
     * 生命周期回调：组件结束运行时调用
     *
     * 负责清理运行时创建的碰撞组件，并重置绑定引用
     *
     * @param EndPlayReason 结束原因
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /**
     * Tick 回调
     *
     * 当 bDebugDraw=true 且碰撞组件存在时，绘制 HurtBox 的调试框
     *
     * @param DeltaTime 帧时间
     * @param TickType Tick 类型
     * @param ThisTickFunction Tick 函数信息
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    /**
     * 获取 Owner 的 SkeletalMeshComponent
     *
     * @return 若 Owner 为 ACharacter 则返回其 Mesh，否则返回 nullptr
     */
    USkeletalMeshComponent* ResolveOwnerMesh() const;

    /**
     * 创建或更新 HurtBox 的底层碰撞组件
     *
     * 行为：
     * - 若 CollisionBox 尚未创建，则在 Owner 上 NewObject<UBoxComponent> 并注册为实例组件
     * - 挂接到 Owner 的 SkeletalMesh
     * - 设置 QueryOnly / Overlap 的碰撞配置
     * - 缓存 BoundComponent 供外部快速匹配
     */
    void CreateOrUpdateCollision();

private:
    /**
     * HurtBox 的底层碰撞盒组件
     *
     * 用于承载实际 Overlap 判定
     */
    UPROPERTY(Transient)
    TObjectPtr<UBoxComponent> CollisionBox = nullptr;

    /**
     * 绑定的碰撞组件弱引用缓存
     *
     * 用于在受击结算时判断命中的组件是否为本 HurtBox 的碰撞体
     */
    UPROPERTY(Transient)
    TWeakObjectPtr<UPrimitiveComponent> BoundComponent;
};
