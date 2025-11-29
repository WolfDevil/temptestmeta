// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "T01/Core/Subsystem/Meta/Data/Node/EMetaGame_MapNodeState.h"
#include "T01/Core/Subsystem/Meta/Data/Node/MetaGame_MapNodeData.h"
#include "MetaMapNodeWidget.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnNodeWidgetClicked);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNodeWidgetClickedDynamic);

UCLASS()
class T01_API UMetaMapNodeWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void Set(const FMetaGame_MapNodeData& Data, EMetaGame_MapNodeState State, bool IsRequiredForTurn);

};
