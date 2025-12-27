#pragma once

#include "CoreMinimal.h"
#include "Character/BMCharacterBase.h"
#include "Character/Components/BMExperienceComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Core/BMTypes.h"
#include "BMPlayerCharacter.generated.h"


class UInputComponent;
class UAnimSequence;
class USpringArmComponent;
class UCameraComponent;
class UBMInventoryComponent;

UCLASS()
/**
 * 玩家角色基类，负责处理玩家输入、视角控制、状态机初始化以及
 * 纯 C++ 驱动的角色动画
 * 
 */
class BLACKMYTH_API ABMPlayerCharacter : public ABMCharacterBase
{
    GENERATED_BODY()

public:
    /**
     * 构造函数
     *
     * 初始化相机组件、移动组件参数以及默认的身份属性，
     * 并为动画播放做好准备
     */
    ABMPlayerCharacter();

    /**
     * 角色正式进入关卡后的初始化逻辑。
     *
     * 完成状态机状态注册、Combat 事件绑定以及初始待机动画的播放等
     */
    virtual void BeginPlay() override;

    /**
     * 输入绑定
     *
     * 将移动、跳跃、攻击以及视角旋转等输入映射绑定到对应的处理函数
     * 
     * @param PlayerInputComponent 用于绑定轴映射和动作映射的输入组件
     */
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "BM|Inventory")
    UBMInventoryComponent* GetInventory() const { return Inventory; }

    /**
     * 落地回调
     *
     * 在角色从空中落到地面时触发，用于清理跳跃状态标记并根据移动意图
     * 切换回 Idle 或 Move 状态
     *
     * @param Hit 描述落地碰撞信息的命中结果
     */
    virtual void Landed(const FHitResult& Hit) override;

    /**
     * 判断当前是否存在有效的移动意图
     *
     * 通常由状态机在 Idle/Move 状态中使用，用于决定是否需要切换到移动状态
     *
     * @return 若当前输入产生的移动向量非零则返回 true，否则返回 false。
     */
    bool HasMoveIntent() const { return MoveIntent.SizeSquared() > KINDA_SMALL_NUMBER; }

    /**
     * 消费一次待处理的起跳标记
     *
     * 由跳跃状态在进入时调用，用于区分玩家主动按键起跳和
     * 被动下落情况
     *
     * @return 若本次存在尚未消费的起跳请求则返回 true，并清除标记；否则返回 false
     */
    bool ConsumePendingJump()
    {
        const bool b = bPendingJump;
        bPendingJump = false;
        return b;
    }

    void ClearActiveAttackSpec();
    /**
     * 判断当前攻击是否应该被打断
     *
     * 根据当前攻击的可打断属性、打断概率以及受到的伤害类型，
     * 决定是否应该中断当前攻击动作并切换到受击状态
     *
     * @param Incoming 收到的伤害信息，包含伤害类型、来源等数据
     * @return 若应当打断当前攻击则返回 true，否则返回 false
     */
    bool ShouldInterruptCurrentAttack(const FBMDamageInfo& Incoming) const;

    /**
     * 解析攻击盒窗口参数
     *
     * 根据窗口ID查询当前激活的攻击盒名称列表及其激活参数，
     * 供动画通知系统在播放攻击动画时激活对应的HitBox组件
     *
     * @param WindowId 窗口标识符
     * @param OutHitBoxNames 返回需要激活的HitBox名称列表
     * @param OutParams 返回HitBox激活参数
     * @return 若成功解析窗口并返回有效的HitBox列表则返回 true，否则返回 false
     */
    virtual bool ResolveHitBoxWindow(
        FName WindowId,
        TArray<FName>& OutHitBoxNames,
        FBMHitBoxActivationParams& OutParams
    ) const override;

    /**
     * 设置当前攻击的上下文信息
     *
     * 在进入攻击状态时调用，记录本次攻击使用的HitBox、激活参数以及打断规则，
     * 用于后续受击判定和动画通知系统查询
     *
     * @param HitBoxNames 本次攻击激活的HitBox名称列表
     * @param Params HitBox激活参数
     * @param bUninterruptible 是否为不可打断攻击
     * @param InterruptChance 普通攻击的打断概率，范围 [0.0, 1.0]
     * @param InterruptChanceOnHeavyHit 重攻击的打断概率，范围 [0.0, 1.0]
     */
    void SetActiveAttackContext(
        const TArray<FName>& HitBoxNames,
        const FBMHitBoxActivationParams& Params,
        bool bUninterruptible,
        float InterruptChance,
        float InterruptChanceOnHeavyHit
    );

    /**
     * 清除当前攻击上下文
     *
     * 在攻击动作结束或被打断时调用，重置所有攻击相关的上下文数据
     */
    void ClearActiveAttackContext();


    /**
     * 查询技能配置
     *
     * 根据战斗动作类型查找对应的技能配置，返回技能的攻击规格和耐力消耗
     *
     * @param Action 战斗动作类型
     * @param OutSpec 输出参数，返回技能的攻击规格
     * @param OutStaminaCost 输出参数，返回释放该技能所需的耐力值
     * @return 若找到对应技能配置且动画资源有效则返回 true，否则返回 false
     */
    bool SelectSkillSpec(EBMCombatAction Action, FBMPlayerAttackSpec& OutSpec, float& OutStaminaCost) const;

    /**
     * 获取普通攻击连段的总步数
     *
     * @return 连段配置数组的长度
     */
    int32 GetComboStepCount() const { return NormalComboSteps.Num(); }

    /**
     * 获取指定索引的连段配置
     *
     * 根据连段步骤索引查询对应的连段配置信息，包括动画、HitBox、打断规则等
     *
     * @param Index 连段步骤索引，范围 [0, GetComboStepCount()-1]
     * @param Out 输出参数，返回连段配置数据
     * @return 若索引有效且动画资源存在则返回 true，否则返回 false
     */
    bool GetComboStep(int32 Index, FBMPlayerComboStep& Out) const;
    /**
     * 获取经验组件
     *
     * @return 玩家角色的经验组件指针，用于管理等级、经验值等
     */
    UBMExperienceComponent* GetExperience() const { return Experience; }

    /**
     * 播放连段收招动画
     *
     * 在连段攻击结束后播放收招动画，用于平滑过渡到待机或移动状态
     *
     * @param Step 连段步骤配置，包含收招动画、播放速率等参数
     * @return 收招动画的实际播放时长（秒），若无收招动画则返回 0
     */
    float PlayComboRecoverOnce(const FBMPlayerComboStep& Step);


    /**
     * 播放待机循环动画。
     */
    void PlayIdleLoop();

    /**
     * 播放移动/跑步循环动画
     */
    void PlayMoveLoop();

    /**
     * 播放通用攻击动画
     *
     * 根据攻击规格配置播放对应的攻击动画，适用于技能和连段攻击
     *
     * @param Spec 攻击规格，包含动画序列、播放速率、起始时间、最大播放时长等参数
     * @return 攻击动画的实际播放时长（秒），若动画不存在则返回 0
     */
    float PlayAttackOnce(const FBMPlayerAttackSpec& Spec);

    /**
     * 播放普通攻击动画
     *
     * 播放一次性的普通攻击动作，支持自定义播放参数
     *
     * @param Seq 攻击动画序列指针
     * @param PlayRate 动画播放速率，默认为 1.0
     * @param StartTime 动画起始时间（秒），默认从头开始
     * @param MaxPlayTime 最大播放时长（秒），默认 -1 表示播放完整动画
     * @return 动画实际播放时长（秒），若动画不存在则返回 0
     */
    float PlayNormalAttackOnce(UAnimSequence* Seq,
        float PlayRate = 1.0f,
        float StartTime = 0.0f,
        float MaxPlayTime = -1.0f);

    /**
     * 播放受击动画
     *
     * 根据受到的伤害信息选择对应的受击动画（轻/重），播放一次
     *
     * @param Info 伤害信息，用于判断受击类型（轻击/重击）并选择合适的动画
     * @return 受击动画的实际播放时长（秒），若动画不存在则返回 0
     */
    float PlayHitOnce(const FBMDamageInfo& Info);

    /**
     * 播放死亡动画
     *
     * 角色死亡时播放的一次性死亡动作
     *
     * @return 死亡动画的播放时长（秒），若动画不存在则返回 0
     */
    float PlayDeathOnce();

    /**
     * 播放闪避动画
     *
     * 执行闪避动作时播放的快速翻滚或闪身动画
     *
     * @return 闪避动画的实际播放时长（秒），若动画不存在则返回 0
     */
    float PlayDodgeOnce();

    /**
     * 计算并锁定闪避方向
     *
     * 根据当前输入方向或角色朝向计算闪避的移动方向，
     * 优先使用玩家输入方向，若无输入则使用角色当前朝向
     *
     * @return 归一化的闪避方向向量
     */
    FVector ComputeDodgeDirectionLocked() const;
    /**
     * 播放一次性的起跳动画
     *
     * 一般在从地面进入 Jump 状态时调用，播放起跳过渡，并配合 Jump() 完成起跳
     *
     * @param PlayRate 动画播放速率，默认为 1.0
     * @return 动画实际播放时长（秒），若动画不存在则返回 0
     */
    float PlayJumpStartOnce(float PlayRate = 1.0f);

    /**
     * 播放空中下落循环动画
     *
     * 在起跳动画结束后仍处于空中，或从平台边缘掉落时调用，
     * 表现持续下落的空中动作
     */
    void  PlayFallLoop();

    /**
     * 将战斗动作加入输入队列
     *
     * 当玩家在攻击过程中再次按下攻击键时，将新的动作请求加入队列，
     * 用于实现连段输入缓冲机制
     *
     * @param Action 要加入队列的战斗动作类型
     */
    void EnqueueAction(EBMCombatAction Action);

    /**
     * 消费下一个队列中的战斗动作
     *
     * 从输入队列头部取出并移除一个动作请求，用于状态机处理缓冲的输入
     *
     * @param OutAction 输出参数，返回队列中的下一个动作
     * @return 若队列非空且成功消费一个动作则返回 true，否则返回 false
     */
    bool ConsumeNextQueuedAction(EBMCombatAction& OutAction);

    /**
     * 消费队列中的一次普通攻击
     *
     * 从输入队列中查找并移除第一个普通攻击动作，用于连段系统
     *
     * @return 若队列中存在普通攻击并成功移除则返回 true，否则返回 false
     */
    bool ConsumeOneQueuedNormalAttack();

    /**
     * 判断是否正在冲刺
     *
     * @return 若当前按住冲刺键则返回 true，否则返回 false
     */
    bool IsSprinting() const { return bSprintHeld; }

    /**
     * 应用步态设置
     *
     * 根据当前是否冲刺切换角色的移动速度和对应的移动动画
     */
    void ApplyGait();
	FBMDamageInfo GetLastDamageInfo() const { return LastDamageInfo; }

    UPROPERTY(EditAnywhere, Category = "BM|Player|Gait")
    float WalkSpeed = 350.f;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Gait")
    float RunSpeed = 600.f;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Dodge")
    float DodgeCooldown = 1.2f;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Dodge")
    float DodgeSpeed = 900.f;

    UPROPERTY(EditAnywhere, Category = "BM|Dodge")
    float DodgeDistance = 420.f;   

    UPROPERTY(EditAnywhere, Category = "BM|Player|Dodge")
    float DodgePlayRate = 1.0f;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Dodge")
    FName DodgeCooldownKey = TEXT("Player_Dodge");

    UPROPERTY(Transient)
    FVector DodgeLockedDir = FVector::ForwardVector;
protected:
/**
 * 处理受到伤害
 *
 * 当角色受到伤害时触发，根据当前状态和伤害信息决定是否切换到受击状态，
 * 或仅扣除生命值而不播放受击动画
 *
 * @param FinalInfo 最终计算后的伤害信息
 */
virtual void HandleDamageTaken(const FBMDamageInfo& FinalInfo) override;

/**
 * 处理角色死亡
 *
 * 当角色生命值降至0时触发，切换到死亡状态，播放死亡动画，
 * 停止关卡音乐并播放死亡音效
 *
 * @param LastHitInfo 导致死亡的最后一次伤害信息
 */
virtual void HandleDeath(const FBMDamageInfo& LastHitInfo) override;

private:
/**
 * 处理前后移动轴输入
 *
 * 根据摄像机朝向计算世界空间前向向量，并更新 MoveIntent.X，
 * 随后由 UpdateMoveIntent() 统一应用移动输入
 *
 * @param Value 轴输入值
 */
void Input_MoveForward(float Value);

/**
 * 处理冲刺键按下
 *
 * 设置冲刺状态标记，切换角色移动速度和动画到冲刺模式
 */
void Input_SprintPressed();

/**
 * 处理冲刺键释放
 *
 * 清除冲刺状态标记，恢复角色移动速度和动画到正常行走模式
 */
void Input_SprintReleased();

    /**
     * 处理左右移动轴输入
     *
     * 根据摄像机朝向计算世界空间右向向量，并更新 MoveIntent.Y，
     * 随后由 UpdateMoveIntent() 统一应用移动输入
     *
     * @param Value 轴输入值
     */
    void Input_MoveRight(float Value);

    /**
     * 处理跳跃键按下
     *
     * 在地面且允许执行动作时设置待起跳标记，并通过状态机切换到跳跃状态
     */
    void Input_JumpPressed();

    /**
     * 处理闪避键按下
     *
     * 检查冷却时间和耐力是否充足，若满足条件则通过 Combat 组件请求闪避动作
     */
    void Input_DodgePressed();

    /**
     * 处理普通攻击键按下
     *
     * 通过 Combat 组件请求普通攻击动作，会被加入输入队列用于连段系统
     */
    void Input_NormalAttackPressed();

    /**
     * 处理技能1键按下
     *
     * 通过 Combat 组件请求技能1释放，需满足冷却和耐力条件
     */
    void Input_Skill1Pressed();

    /**
     * 处理技能2键按下
     *
     * 通过 Combat 组件请求技能2释放，需满足冷却和耐力条件
     */
    void Input_Skill2Pressed();

    /**
     * 处理技能3键按下
     *
     * 通过 Combat 组件请求技能3释放，需满足冷却和耐力条件
     */
    void Input_Skill3Pressed();

    /**
     * 水平视角旋转输入回调
     *
     * 将 MouseX 水平输入转换为控制器旋转，
     * 驱动 SpringArm 和相机围绕角色旋转
     *
     * @param Value 轴输入值，通常来自鼠标 X 轴移动，正值向右、负值向左
     */
    void Input_Turn(float Value);

    /**
     * 垂直视角旋转输入回调
     *
     * 将 MouseY 垂直输入转换为控制器 Pitch 旋转，
     * 控制相机抬头与低头角度
     *
     * @param Value 轴输入值，通常来自鼠标 Y 轴移动，正值向上、负值向下
     */
    void Input_LookUp(float Value);

    /**
     * 切换背包UI显示
     *
     * 按下 I 键时调用，打开或关闭背包界面
     */
    void Input_ToggleInventory();

    /**
     * 切换自动增加货币测试功能
     *
     * 调试功能，按下 M 键时启用/禁用自动增加金币的测试模式
     */
    void Input_ToggleAutoAddCurrencyTest();

    /**
     * 切换强制物品价格为10的测试功能
     *
     * 调试功能，按下 N 键时启用/禁用所有物品价格固定为10的测试模式
     */
	void Input_ToggleForcePrice10Test();

    /**
     * 快捷栏槽位1输入处理
     *
     * 按下数字键1时触发，调用 TriggerHotbarSlot(1)
     */
    void HotbarSlot1();

    /**
     * 快捷栏槽位2输入处理
     *
     * 按下数字键2时触发，调用 TriggerHotbarSlot(2)
     */
    void HotbarSlot2();

    /**
     * 快捷栏槽位3输入处理
     *
     * 按下数字键3时触发，调用 TriggerHotbarSlot(3)
     */
    void HotbarSlot3();

    /**
     * 快捷栏槽位4输入处理
     *
     * 按下数字键4时触发，调用 TriggerHotbarSlot(4)
     */
    void HotbarSlot4();

    /**
     * 快捷栏槽位5输入处理
     *
     * 按下数字键5时触发，调用 TriggerHotbarSlot(5)
     */
    void HotbarSlot5();

    /**
     * 快捷栏槽位6输入处理
     *
     * 按下数字键6时触发，调用 TriggerHotbarSlot(6)
     */
    void HotbarSlot6();

    /**
     * 快捷栏槽位7输入处理
     *
     * 按下数字键7时触发，调用 TriggerHotbarSlot(7)
     */
    void HotbarSlot7();

    /**
     * 快捷栏槽位8输入处理
     *
     * 按下数字键8时触发，调用 TriggerHotbarSlot(8)
     */
    void HotbarSlot8();

    /**
     * 快捷栏槽位9输入处理
     *
     * 按下数字键9时触发，调用 TriggerHotbarSlot(9)
     */
    void HotbarSlot9();

public:
   /**
    * 触发快捷栏槽位
    *
    * 根据槽位索引触发对应的快捷栏功能，背包打开时购买物品，
    * 背包关闭时使用物品
    *
    * @param SlotIndex 快捷栏槽位索引，范围 [1, 9]，对应数字键 1-9
    */
void TriggerHotbarSlot(int32 SlotIndex);

    /**
     * 初始化并注册玩家相关的状态机状态
     *
     * 在 BeginPlay 中调用，创建 Idle/Move/Jump/Attack 等状态实例，
     * 并注册到 UBMStateMachineComponent 中，同时设置初始状态
     */
    void InitFSMStates();

    /**
     * 处理战斗动作请求
     *
     * 当 Combat 组件接收到动作请求时触发的回调函数，
     * 根据动作类型和当前状态决定是否切换状态或加入输入队列
     *
     * @param Action 请求的战斗动作类型
     */
    void OnActionRequested(EBMCombatAction Action);

    /**
     * 根据当前 MoveIntent 向量与控制器朝向应用移动输入
     *
     * 在输入回调中更新 MoveIntent 后调用，内部会检查 Combat 是否允许动作，
     * 并通过 AddMovementInput 驱动角色移动
     */
    void UpdateMoveIntent();

    /**
     * 以循环方式播放给定动画
     *
     * 使用 AnimationSingleNode 模式在角色 Mesh 上播放指定的 UAnimSequence，
     * 若与当前循环动画相同则不会重复设置
     *
     * @param Seq 需要循环播放的动画序列指针，可以为 nullptr；为空时不执行任何操作
     */
    void PlayLoop(UAnimSequence* Seq);

    /**
     * 工具函数：以非循环方式播放一次动画
     *
     * 使用 AnimationSingleNode 模式播放一次性动作，并允许指定起始时间与最大播放时长：
     * - StartTime 用于从动画中间开始播放
     * - MaxPlayTime 用于限制本次播放的有效时长
     *
     * 函数会返回本次有效播放时长，供状态机设置计时器或做状态切换。
     *
     * @param Seq         需要播放的动画序列指针；若为空则不执行播放
     * @param PlayRate    动画播放速率，1.0 为原速；值越大动画越快
     * @param StartTime   播放起始时间（秒），默认 0.0
     * @param MaxPlayTime 本次最大播放时长（秒），默认 -1
     *
     * @return 本次动画实际有效播放时长（秒），若动画不存在则返回 0
     */
    float PlayOnce(
        UAnimSequence* Seq,
        float PlayRate = 1.0f,
        float StartTime = 0.0f,
        float MaxPlayTime = -1.0f
    );

    /**
     * 构建攻击步骤配置
     *
     * 在构造函数中调用，初始化普通攻击连段配置和技能配置数组，
     * 设置每个攻击步骤的动画、HitBox、打断规则等参数
     */
    void BuildAttackSteps();

private:
    /**
     * 设置相机组件
     *
     * 在构造函数中调用，初始化 CameraBoom 和 FollowCamera
     */
    void SetupCamera();

    /**
     * 设置角色网格体
     *
     * 在构造函数中调用，加载并配置角色的 SkeletalMesh
     */
    void SetupMesh();

    /**
     * 设置动画资源
     *
     * 在构造函数中调用，加载所有动画序列资源
     */
    void SetupAnimations();

    /**
     * 设置受击盒组件
     *
     * 在构造函数中调用，创建并配置 HurtBox 组件
     */
    void SetupHurtBoxes();

    /**
     * 设置攻击盒组件
     *
     * 在构造函数中调用，注册 HitBox 定义
     */
    void SetupHitBoxes();

public:
    /**
     * 相机臂组件
     *
     * 用于将相机与角色保持一定距离，并跟随控制器旋转，实现第三人称跟随视角。
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    /**
     * 跟随相机组件
     *
     * 附着在 CameraBoom 末端，实际负责渲染玩家视角
     */
    UPROPERTY(VisibleAnywhere, Category = "BM|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    /**
     * 玩家角色使用的骨骼网格资产
     *
     * 可以在构造函数中通过路径加载
     */
    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<USkeletalMesh> CharacterMeshAsset;

    /**
     * 待机循环动画序列
     *
     * 在 Idle 状态下使用，表现角色静止时的呼吸等动作
     */
    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimIdle;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Gait")
    TObjectPtr<UAnimSequence> AnimWalk = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Gait")
    TObjectPtr<UAnimSequence> AnimRun = nullptr;

    // === 普通攻击连段 ===
    UPROPERTY(EditAnywhere, Category = "BM|Player|Combo")
    TArray<FBMPlayerComboStep> NormalComboSteps;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Combo")
    float ComboRecoverPlayRate = 1.0f;

    // === 技能列表 ===
    UPROPERTY(EditAnywhere, Category = "BM|Player|Skill")
    TArray<FBMPlayerSkillSlot> SkillSlots;

    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimDodge;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttack1 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttack2 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttack3 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttack4 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttackRecover1 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttackRecover2 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttackRecover3 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimNormalAttackRecover4 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimSkill1 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimSkill2 = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimSkill3 = nullptr;
    // 受击/死亡动画
    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimHitLight = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimHitHeavy = nullptr;

    UPROPERTY(EditAnywhere, Category = "BM|Player|Assets")
    TObjectPtr<UAnimSequence> AnimDeath = nullptr;

    // 输入缓冲：支持连段多次按键
    UPROPERTY(Transient)
    TArray<EBMCombatAction> ActionQueue;

    // 当前攻击上下文
    bool bHasActiveAttackContext = false;

    TArray<FName> ActiveHitBoxNames;
    FBMHitBoxActivationParams ActiveHitBoxParams;

    bool bActiveUninterruptible = false;
    float ActiveInterruptChance = 0.f;
    float ActiveInterruptChanceOnHeavyHit = 0.f;

    FBMDamageInfo LastDamageInfo; 

    /**
     * 待起跳标记
     *
     * 在接收到地面跳跃输入时置为 true，由 Jump 状态在进入时消费，
     * 用于区分主动起跳与被动掉落
     */
    bool bPendingJump = false;

    /**
     * 起跳动画序列（JumpStart）
     *
     * 在从地面起跳时播放的一次性过渡动画
     */
    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimJumpStart = nullptr;

    /**
     * 下落循环动画序列（FallLoop）
     *
     * 在空中下落阶段持续播放，无论是起跳后的滞空还是从平台边缘掉落
     */
    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimFallLoop = nullptr;

private:
    /**
     * 当前的移动意图向量
     *
     * X 表示前后输入，Y 表示左右输入，范围通常在 [-1, 1]
     * 由输入回调更新，再由 UpdateMoveIntent() 统一应用
     */
    FVector2D MoveIntent = FVector2D::ZeroVector;

    bool bSprintHeld = false;

    /**
     * 当前正在播放的循环动画引用
     *
     * 用于避免重复设置同一 UAnimSequence
     */
    UPROPERTY(Transient)
    TObjectPtr<UAnimSequence> CurrentLoopAnim = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMInventoryComponent> Inventory;

    UPROPERTY(VisibleAnywhere, Category = "BM|Components")
    TObjectPtr<UBMExperienceComponent> Experience;
};

