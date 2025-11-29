// Fill out your copyright notice in the Description page of Project Settings.


#include "AMetaGame_MapNodeActor.h"

#include "T01/Core/Subsystem/Meta/Widgets/MetaMapNodeWidget.h"


AMetaGame_MapNodeActor::AMetaGame_MapNodeActor()
{
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	RootComponent = WidgetComponent;
	WidgetComponent->SetDrawSize(FVector2D(WidgetDefaultSize, WidgetDefaultSize));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMetaGame_MapNodeActor::InitNode(const FMetaGame_MapNodeData* InData, EMetaGame_MapNodeState InState, bool InIsRequiredForTurn)
{
	NodeDataPtr = InData;
	NodeState = InState;
	IsRequiredForTurn = InIsRequiredForTurn;

	SetActorLocation(InData->WorldPosition);
	UpdateNode();
}

void AMetaGame_MapNodeActor::SetState(EMetaGame_MapNodeState InState)
{
	NodeState = InState;
	UpdateNode();
}

void AMetaGame_MapNodeActor::UpdateNode()
{
	if (auto* Widget = WidgetComponent->GetUserWidgetObject())
	{
		NodeWidget = Cast<UMetaMapNodeWidget>(Widget);
		if (NodeWidget && NodeDataPtr)
		{
			NodeWidget->Set(*NodeDataPtr, NodeState, IsRequiredForTurn);
		}
	}
}

void AMetaGame_MapNodeActor::UpdateScale(float NewMultiplier)
{
	WidgetComponent->SetDrawSize(FVector2D(WidgetDefaultSize * NewMultiplier, WidgetDefaultSize * NewMultiplier));
}
