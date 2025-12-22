#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/BMSkillData.h"
#include "BMSkillComponent.generated.h"

class ABMPlayerCharacter;
class UBMDataSubsystem;
class UBMStatsComponent;

/**
 * 技能系统日志分类
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBMSkill, Log, All);

/**
 * 技能执行完成事件
 * 
 * 当技能执行完成时触发，可用于通知其他系统（如状态机、UI等）
 * 
 * 参数说明：
 * - SkillID: 技能的唯一标识符
 * - bSuccess: 技能是否成功执行
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FBMOnSkillExecuted, FName /*SkillID*/, bool /*bSuccess*/);

/**
 * 技能组件（BMSkillComponent）
 * 
 * 负责管理单个技能实例的运行时状态和行为逻辑。
 * 每个技能实例对应一个 BMSkillComponent，通过 SkillID 从数据子系统获取配置数据。
 * 
 * 主要功能：
 * - 技能冷却时间管理
 * - 资源消耗检查与扣除（MP/Stamina）
 * - 技能可用性判断
 * - 技能执行逻辑
 * 
 * 设计说明：
 * - 数据驱动：技能数值（伤害、冷却、消耗等）存储在 FBMSkillData 中
 * - 状态管理：维护技能的上次使用时间、激活状态等运行时信息
 * - 资源管理：与 Stats 组件协作，检查并消耗 MP 或 Stamina
 */
UCLASS(ClassGroup = (BM), meta = (BlueprintSpawnableComponent = "false"))
class BLACKMYTH_API UBMSkillComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     * 
     * 初始化组件的默认值，不启用 Tick
     */
    UBMSkillComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /**
     * 游戏开始时调用
     * 
     * 从数据子系统加载技能配置数据
     */
    virtual void BeginPlay() override;

    /**
     * 初始化技能组件
     * 
     * 设置技能ID并从数据子系统加载对应的技能数据。
     * 必须在 BeginPlay 之后调用，确保数据子系统已初始化。
     * 
     * @param InSkillID 技能的唯一标识符（对应数据表中的行名）
     * @return 如果成功加载技能数据返回 true，否则返回 false
     */
    bool InitializeSkill(FName InSkillID);

    /**
     * 执行技能
     * 
     * 技能释放的核心函数。在执行前会进行可用性检查（冷却、资源、激活状态），
     * 检查通过后扣除资源、更新使用时间、设置激活状态，并触发执行逻辑。
     * 
     * 注意：此函数只负责技能的基础逻辑（资源扣除、状态更新），
     * 具体的技能效果（如播放动画、产生伤害等）应由外部系统（如状态机、Combat组件）处理。
     * 
     * @param PlayerCharacter 释放技能的玩家角色指针
     * @return 如果技能成功执行返回 true，否则返回 false
     */
    bool Execute(ABMPlayerCharacter* PlayerCharacter);

    /**
     * 获取技能消耗的资源量
     * 
     * 从技能配置数据中读取消耗值。
     * 
     * @return 技能消耗的资源量（MP 或 Stamina），如果配置无效返回 0.0f
     */
    float GetCost() const;

    /**
     * 获取技能的完整冷却时间
     * 
     * 从技能配置数据中读取冷却时间。
     * 
     * @return 技能的冷却时间（秒），如果配置无效返回 0.0f
     */
    float GetCooldown() const;

    /**
     * 检查技能当前是否可用
     * 
     * 综合判断技能的可用性，包括：
     * 1. 技能配置是否有效
     * 2. 是否处于冷却中（当前时间 - LastUsedTime < Cooldown）
     * 3. 资源是否足够（当前 MP >= Cost，假设使用 MP）
     * 4. 是否已经处于激活状态（bIsSkillActive）
     * 
     * @return 如果技能可用返回 true，否则返回 false
     */
    bool IsAvailable() const;

    /**
     * 获取剩余的冷却时间
     * 
     * 计算技能还需要多长时间才能再次使用。
     * 主要用于 UI 显示（例如技能图标上的倒计时数字）。
     * 
     * @return 剩余的冷却时间（秒），如果不在冷却中或配置无效返回 0.0f
     */
    float GetRemainingCooldown() const;

    /**
     * 获取技能ID
     * 
     * @return 当前技能的唯一标识符
     */
    FName GetSkillID() const { return SkillID; }

    /**
     * 获取技能配置数据
     * 
     * @return 技能配置数据的只读指针，如果未加载返回 nullptr
     */
    const FBMSkillData* GetSkillData() const { return SkillData; }

    /**
     * 检查技能是否处于激活状态
     * 
     * @return 如果技能正在执行中返回 true，否则返回 false
     */
    bool IsSkillActive() const { return bIsSkillActive; }

    /**
     * 设置技能的激活状态
     * 
     * 通常在技能开始执行时设置为 true，执行完成时设置为 false。
     * 用于防止持续性技能在执行期间被重复释放。
     * 
     * @param bActive 新的激活状态
     */
    void SetSkillActive(bool bActive) { bIsSkillActive = bActive; }

    /**
     * 技能执行完成事件
     * 
     * 外部系统可以绑定此事件来响应技能的执行结果。
     */
    FBMOnSkillExecuted OnSkillExecuted;

protected:
    /**
     * 技能的唯一标识符
     * 
     * 用于从数据子系统（BMDataSubsystem）中查找对应的技能配置数据。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BM|Skill")
    FName SkillID = NAME_None;

    /**
     * 技能配置数据的缓存指针
     * 
     * 在 InitializeSkill 时从数据子系统加载，避免每次访问都查询数据表。
     * 注意：此指针指向的数据由数据子系统管理，不应手动释放。
     * 
     * 这是运行时缓存数据，不需要序列化或反射。
     */
    const FBMSkillData* SkillData = nullptr;

    /**
     * 技能上一次被释放的时间点
     * 
     * 记录技能上次使用时的游戏世界时间（GetWorld()->GetTimeSeconds()）。
     * 用于计算冷却时间，判断技能是否可以再次使用。
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BM|Skill")
    float LastUsedTime = 0.0f;

    /**
     * 技能当前是否处于激活状态
     * 
     * 标记技能是否正在执行中。用于处理持续性技能或防止在动作未完成时重复释放。
     * 例如：一个需要持续施法的技能在执行期间应保持 bIsSkillActive = true，
     * 直到技能完成或被打断时才设置为 false。
     * 
     * 注意：使用 bIsSkillActive 而不是 bIsActive，因为 UActorComponent 基类已经定义了 bIsActive。
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BM|Skill")
    bool bIsSkillActive = false;

private:
    /**
     * 检查资源是否足够
     * 
     * 根据技能配置的消耗值，检查角色的 MP 是否足够。
     * 注意：当前实现假设所有技能都消耗 MP，如需支持 Stamina 或其他资源类型，
     * 可以在 FBMSkillData 中添加资源类型字段。
     * 
     * @param PlayerCharacter 要检查的玩家角色
     * @return 如果资源足够返回 true，否则返回 false
     */
    bool CheckResourceAvailable(ABMPlayerCharacter* PlayerCharacter) const;

    /**
     * 消耗技能所需的资源
     * 
     * 从角色的 Stats 组件中扣除技能消耗的资源量。
     * 
     * @param PlayerCharacter 要扣除资源的玩家角色
     * @return 如果成功扣除资源返回 true，否则返回 false
     */
    bool ConsumeResource(ABMPlayerCharacter* PlayerCharacter) const;
};

