#Lr
FMoverDataStructBase :

FMoverDataStructBase的结构体用于封装所需的输入参数
FMoverDataStructBase 是 Mover 框架中
“可预测、可回滚、可插值、值语义”的最小状态单元。

1. ShouldReconcile
 true:回滚
virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override
{
    const FZipliningState& Auth = static_cast<const FZipliningState&>(AuthorityState);

    return ZiplineActor != Auth.ZiplineActor
        || bIsMovingAtoB != Auth.bIsMovingAtoB;
}

2. Interpolate
 在两帧权威状态之间，我该如何‘平滑显示’？”
virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float LerpFactor) override
{
    const FFollowPathState& A = static_cast<const FFollowPathState&>(From);
    const FFollowPathState& B = static_cast<const FFollowPathState&>(To);

    CurrentPathPos = FMath::Lerp(A.CurrentPathPos, B.CurrentPathPos, Alpha);
}
连续值 → Lerp
离散值 → Alpha < 0.5 ? A : B

3. Merge // ❌ 不实现的后果 Dash / Jump 输入被吞，“有时按了没反应”
Network Prediction 允许一帧内多次 GenerateMove： “同一帧内多次 GenerateMove，这些 State 怎么合并？”
virtual void Merge(const FMoverDataStructBase& From) override
{
	const FMoverExampleAbilityInputs& Src = static_cast<const FMoverExampleAbilityInputs&>(From);
    bIsDashJustPressed |= Src.bIsDashJustPressed;
}


4. Clone 
“当系统需要保存历史状态时，怎么复制你这个 State？”
FMoverDataStructBase* Clone() const override
{
    return new FZipliningState(*this);//必须 deep copy
}

5. NetSerialize 
“这个 State 怎么在网络上传？”
调用时机，服务器 → 客户端，Authority Sync
📌 原则
    ·顺序一致
    ·Bit 精简
    ·UObject 用 Map
bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    Super::NetSerialize(Ar, Map, bOutSuccess);

    Ar << CurrentPathPos;
    Ar.SerializeBits(&bIsMovingAtoB, 1);

    bOutSuccess = true;
    return true;
}

6. GetScriptStruct
“告诉 Mover，这个 DataStruct 的真实类型是什么？”
📌 用于：
    ·类型识别
    ·数据集合查找
    ·Blueprint 支持

UScriptStruct* GetScriptStruct() const override
{
    return StaticStruct();
}

6. ToString
调试 + Network Prediction Log
void FZipliningState::ToString(FAnsiStringBuilderBase& Out) const
{
	Super::ToString(Out);

	Out.Appendf("ZiplineActor: %s\n", *GetNameSafe(ZiplineActor));
	Out.Appendf("IsMovingAtoB: %d\n", bIsMovingAtoB);
}

7. AddReferencedObjects
防止 GC 回收你 State 里引用的 UObject，
什么时候需要？当State 内含 UObject*，且不是 UPROPERTY

void AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(ZiplineActor);
}


# ######################################################################################################################################################
同一个 Simulation Frame （客户端 / 服务器严格一致）内：只会“激活一个 UBaseMovementMode”作为主 Mode 执行，因此不会多个Mode并行


UBaseMovementMode：
Frame N
│
├─ ProduceInput                （输入采集）
│
├─ GenerateMove                （意图 → ProposedMove）
│
├─ SimulationTick (xN 次)      （执行 + 写 State）
│
└─ PhysicsCharacterMover       （物理落地）

同一个 Simulation Frame 内：
GenerateMove 最多一次 / Tick
SimulationTick 可能多次 / Tick

1. GenerateMove_Implementation
virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState,const FMoverTimeStep& TimeStep,FProposedMove& OutProposedMove) const override;

 1) FMoverTickStartData& StartState “这一小段模拟开始前，我是谁？”
    它包含：
    当前 Transform
    当前 Velocity
    当前 MovementMode
    上一帧 Sync / Aux State
    当前 Input Cmd
    📌 只读、快照、不可修改

 2) FMoverTimeStep& TimeStep // TimeStep时间长度，这个世界长度内SimulationTick_Implementation会调用多次
    “我这次算多长时间？”
    DeltaSeconds
    子步信息（Substep index）
    是否回滚帧
    📌 只用它的 DeltaSeconds

简化模型：
float RemainingTime = DeltaTime;
int32 Iterations = 0;

while (RemainingTime > KINDA_SMALL_NUMBER &&
       Iterations < MaxSimulationIterations)
{
    const float TimeStep = ComputeTimeStep(RemainingTime);
    SimulationTick(TimeStep);
    RemainingTime -= TimeStep;
    Iterations++;
}


 3) 🧩 FProposedMove& OutProposedMove “我提议怎么动（不一定真的这么动）”
    你可以写的内容：

2. SimulationTick_Implementation 真正的“世界模拟”,📌 这是你唯一可以写 State 的地方
virtual void SimulationTick_Implementation(const FSimulationTickParams& Params, FMoverTickEndData& OutputState) override;
| 内容                  | 用途              |
| ------------------- | --------------- |
| Params.StartState   | 上一状态            |
| Params.ProposedMove | 来自 GenerateMove |
| Params.TimeStep     | Delta           |
| Params.World        | 查询世界            |
| Params.MovementBase | 站立物             |
| Params.AuxState     | 可写              |
| Params.SyncState    | 可写              |

📌 它可以：
    Trace
    检测落地
    消耗时间
    切换 Mode
    修改 Sync / Aux State

📌 但：
    不能直接 Move Actor
    不能依赖随机数
    不能依赖 Tick 顺序
eg:
“这次模拟结束，我变成什么状态？”
OutputState.SyncState->Set(...)
OutputState.AuxState->Set(...)
OutputState.OutVelocity = ...
OutputState.OutTransform = ...
这些会进入：网络同步,回滚缓存,下一帧 StartState



【Simulation Frame 开始】

① Evaluate_Implementation   （Transitions）
② GenerateMove_Implementation（仅一次）
③ SimulationTick_Implementation（可多次）
   ├── (MovementEndState → Mode 切换)
   ├── Evaluate_Implementation（再次）
   ├── SimulationTick_Implementation（新 Mode）
   └── ...
【Simulation Frame 结束】

# ######################################################################################################################################################
UBaseMovementModeTransition

1. Evaluate_Implementation: 在 1 个 Simulation Frame 内可能会被 多次 Evaluate，但它最多只会成功触发一次 Mode 切换
	virtual FTransitionEvalResult Evaluate_Implementation(const FSimulationTickParams& Params) const override;
2. Trigger_Implementation 切换时触发
	virtual void Trigger_Implementation(const FSimulationTickParams& Params) override;
