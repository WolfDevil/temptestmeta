// Fill out your copyright notice in the Description page of Project Settings.


#include "AMetaGame_MapNodeActor.h"

#include "Kismet/GameplayStatics.h"
#include "T01/Core/Subsystem/Meta/MetaGameSubsystem.h"
#include "T01/Core/Subsystem/Meta/Widgets/MetaMapNodeWidget.h"


AMetaGame_MapNodeActor::AMetaGame_MapNodeActor()
{
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	RootComponent = WidgetComponent;
	WidgetComponent->SetDrawSize(FVector2D(WidgetDefaiultSize, WidgetDefaiultSize));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMetaGame_MapNodeActor::InitNode(const FMetaGame_MapNodeData& InData, EMetaGame_MapNodeState InState, bool InIsRequiredForTurn)
{
	NodeData = InData;
	NodeState = InState;
	IsRequiredForTurn = InIsRequiredForTurn;

	SetActorLocation(InData.WorldPosition);
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
		if (NodeWidget)
		{
			NodeWidget->Set(NodeData, NodeState, IsRequiredForTurn);


			NodeWidget->OnNodeClicked.Clear();

			TWeakObjectPtr<AMetaGame_MapNodeActor> WeakThis(this);

			NodeWidget->OnNodeClicked.AddLambda([WeakThis](FName ID)
				{
					if (!WeakThis.IsValid())
						return;

					WeakThis->OnClicked();
				}
			);
		}
	}
}

void AMetaGame_MapNodeActor::UpdateScale(float NewMultiplier)
{
	WidgetComponent->SetDrawSize(FVector2D(WidgetDefaiultSize * NewMultiplier, WidgetDefaiultSize * NewMultiplier));
}

void AMetaGame_MapNodeActor::OnClicked()
{
	GetWorld()->GetSubsystem<UMetaGameSubsystem>()->OnNodeClicked(GetNodeID());
}
