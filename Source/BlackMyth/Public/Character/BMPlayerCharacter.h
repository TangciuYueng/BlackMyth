#pragma once

#include "CoreMinimal.h"
#include "Character/BMCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BMPlayerCharacter.generated.h"


class UInputComponent;
class UAnimMontage;
class USpringArmComponent;
class UCameraComponent;

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

    // === 纯C++动画播放接口 ===
    /**
     * 播放待机循环动画。
     *
     * 一般在进入 Idle 状态时调用，使用 AnimationSingleNode 模式循环播放
     */
    void PlayIdleLoop();

    /**
     * 播放移动/跑步循环动画
     *
     * 一般在进入 Move 状态时调用，根据当前速度与移动意图呈现持续移动效果
     */
    void PlayMoveLoop();

    // === 跳跃更新 ===
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
     * 播放一次性的轻攻击动画
     *
     * 由攻击状态调用，使用 AnimationSingleNode 模式播放非循环动画，
     * 播放结束后由状态通过计时器或逻辑回调切换回其他状态
     *
     * @param PlayRate 动画播放速率，默认为 1.0。
     * @return 动画实际播放时长（秒），若动画不存在则返回 0
     */
    float PlayLightAttackOnce(float PlayRate = 1.0f);

protected:
    /**
     * 落地回调
     *
     * 在角色从空中落到地面时触发，用于清理跳跃状态标记并根据移动意图
     * 切换回 Idle 或 Move 状态
     *
     * @param Hit 描述落地碰撞信息的命中结果
     */
    virtual void Landed(const FHitResult& Hit) override;

private:
    /**
     * 处理前后移动轴输入
     *
     * 根据摄像机朝向计算世界空间前向向量，并更新 MoveIntent.X，
     * 随后由 UpdateMoveIntent() 统一应用移动输入
     *
     * @param Value 轴输入值，通常位于 [-1, 1]
     */
    void Input_MoveForward(float Value);

    /**
     * 处理左右移动轴输入
     *
     * 根据摄像机朝向计算世界空间右向向量，并更新 MoveIntent.Y，
     * 随后由 UpdateMoveIntent() 统一应用移动输入
     *
     * @param Value 轴输入值，通常位于 [-1, 1]
     */
    void Input_MoveRight(float Value);

    /**
     * 跳跃按键按下回调
     *
     * 在地面且允许执行动作时，设置待起跳标记 bPendingJump，
     * 并通过状态机切换到 Jump 状态，由 Jump 状态统一触发起跳与动画
     */
    void Input_JumpPressed();

    /**
     * 攻击按键按下回调
     *
     * 将轻攻击请求转交给 Combat 组件，由 Combat 统一校验
     * 能否执行攻击并触发对应的事件
     */
    void Input_AttackPressed();

    /**
     * 水平视角旋转输入回调
     *
     * 将 MouseX 水平输入转换为控制器旋转，
     * 驱动 SpringArm 和相机围绕角色旋转
     *
     * @param Value 轴输入值，通常来自鼠标 X
     */
    void Input_Turn(float Value);

    /**
     * 垂直视角旋转输入回调
     *
     * 将 MouseY 垂直输入转换为控制器 Pitch 旋转，
     * 控制相机抬头与低头角度
     *
     * @param Value 轴输入值，通常来自鼠标 Y
     */
    void Input_LookUp(float Value);

    // Combat 事件：由 Input_AttackPressed -> Combat->RequestLightAttack() 触发
    /**
     * Combat 组件发出的轻攻击请求回调
     *
     * 当玩家攻击输入被 Combat 认可后调用，用于驱动状态机
     * 切换到 Attack 状态
     */
    void OnLightAttackRequested();

    /**
     * 初始化并注册玩家相关的状态机状态
     *
     * 在 BeginPlay 中调用，创建 Idle/Move/Jump/Attack 等状态实例，
     * 并注册到 UBMStateMachineComponent 中，同时设置初始状态
     */
    void InitFSMStates();

    /**
     * 根据当前 MoveIntent 向量与控制器朝向应用移动输入
     *
     * 在输入回调中更新 MoveIntent 后调用，内部会检查 Combat 是否允许动作，
     * 并通过 AddMovementInput 驱动角色移动
     */
    void UpdateMoveIntent();

    /**
     * 工具函数：以循环方式播放给定动画
     *
     * 使用 AnimationSingleNode 模式在角色 Mesh 上播放指定的 UAnimSequence，
     * 若与当前循环动画相同则不会重复设置
     *
     * @param Seq 需要循环播放的动画序列指针，可以为 nullptr
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
     * @param Seq         需要播放的动画序列指针
     * @param PlayRate    动画播放速率，1.0 为原速
     * @param StartTime   播放起始时间（秒），默认 0.0
     * @param MaxPlayTime 本次最大播放时长（秒），默认 -1；<=0 表示播放到动画结尾
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

    /**
     * 移动/跑步循环动画序列
     *
     * 在 Move 状态下使用，表现角色奔跑或移动时的动作
     */
    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimMove;

    /**
     * 轻攻击动画序列
     *
     * 用于基于 AnimationSingleNode 的一次性攻击动作播放
     */
    UPROPERTY(EditAnywhere, Category = "BM|Assets")
    TObjectPtr<UAnimSequence> AnimLightAttack;
    
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

    /**
     * 当前正在播放的循环动画引用
     *
     * 用于避免重复设置同一 UAnimSequence
     */
    UPROPERTY(Transient)
    TObjectPtr<UAnimSequence> CurrentLoopAnim = nullptr;
};

