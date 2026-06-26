// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Component/LrWorldWidgetComponent.h"

ULrWorldWidgetComponent::ULrWorldWidgetComponent()
{
	// 设置Widget的显示空间为屏幕空间。
	// 在世界中放置的Widget组件默认是世界空间，这里强制改为屏幕空间，
	// 意味着Widget将始终面向摄像机，大小以屏幕像素为单位，适合做头顶血条、名字等。
	SetWidgetSpace(EWidgetSpace::Screen);

	// 启用“按期望尺寸绘制”。
	// 当为true时，Widget将按照UMG中设计的Desired Size来绘制，而不是被组件缩放影响。
	// 通常用于保证UI元素在不同距离下保持像素级清晰，不被世界缩放拉伸。
	SetDrawAtDesiredSize(true);

	// 关闭物理碰撞。
	// 对于UI组件来说，通常不需要物理交互，禁用碰撞可避免不必要的性能开销和物理计算。
	BodyInstance.SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 关闭生成重叠事件。
	// 即使有碰撞通道可能产生重叠，也明确禁掉，进一步确保UI不参与游戏逻辑的触发。
	SetGenerateOverlapEvents(false);

	// 设置双面渲染。
	// 默认Widget组件可能只渲染面向摄像机的一面，开启双面后，从背面也能看到Widget。
	// 这对于屏幕空间Widget通常不是必须，但可以防止某些角度下Widget消失的问题。
	SetTwoSided(true);

	// 关闭阴影投射。
	// UI组件不需要投射阴影，关掉以节省渲染开销。
	SetCastShadow(false);

	// 关闭接受贴花。
	// 贴花（如弹孔、血迹）不应投射到UI上，关闭可避免错误的视觉效果和性能浪费。
	SetReceivesDecals(false);

	// EWindowVisibility::SelfHitTestInvisible 表示Widget自身不参与点击测试，
	// 但子控件可以。这行被注释说明当前不需要改变默认的命中测试行为。
	// SetWindowVisibility(EWindowVisibility::SelfHitTestInvisible);


	// 禁用Gamma校正。
	// 在渲染Widget时，默认可能会进行Gamma校正，但UI通常在线性空间下设计，
	// 关闭它可以保证颜色与UMG编辑器中预览的一致，避免颜色偏差。
	bApplyGammaCorrection = false;

	// 设置枢轴点为中心(0.5, 0.5)。
	// 决定了Widget的缩放、旋转和位置参考点。设为中心意味着Widget将以自身中心对齐到组件位置，
	// 符合大多数HUD元素（如血条居中于角色头顶）的需求。
	SetPivot(FVector2D(0.5f, 0.5f));

	// 关闭组件每帧更新。
	// 如果Widget内容不需要动态更新（或者更新由手动驱动），关闭Tick可以节省CPU时间。
	// 若后续需要动态刷新，可在蓝图中重新开启。
	PrimaryComponentTick.bCanEverTick = false;
}

