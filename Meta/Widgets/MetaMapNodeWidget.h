// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "T01/Core/Subsystem/Meta/Data/Node/EMetaGame_MapNodeState.h"
#include "T01/Core/Subsystem/Meta/Data/Node/MetaGame_MapNodeData.h"
#include "MetaMapNodeWidget.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnNodeWidgetClicked, FName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNodeWidgetClickedDynamic, FName, NodeID);

UCLASS()
class T01_API UMetaMapNodeWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void Set(const FMetaGame_MapNodeData& Data, EMetaGame_MapNodeState State, bool IsRequiredForTurn);

	UPROPERTY(BlueprintAssignable)
	FOnNodeWidgetClickedDynamic OnNodeClickedDynamic;
	FOnNodeWidgetClicked OnNodeClicked;

protected:
	UFUNCTION(BlueprintCallable)
	void HandleButtonClicked();

	FMetaGame_MapNodeData NodeData;
	EMetaGame_MapNodeState NodeState;

};
