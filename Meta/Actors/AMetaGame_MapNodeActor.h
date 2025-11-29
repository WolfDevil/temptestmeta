// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "T01/Core/Subsystem/Meta/Data/Node/EMetaGame_MapNodeState.h"
#include "T01/Core/Subsystem/Meta/Data/Node/MetaGame_MapNodeData.h"
#include "AMetaGame_MapNodeActor.generated.h"


UCLASS()
class T01_API AMetaGame_MapNodeActor : public AActor
{
	GENERATED_BODY()

public:
	AMetaGame_MapNodeActor();

	// UFUNCTION(BlueprintCallable)
	void InitNode(const FMetaGame_MapNodeData* InData, EMetaGame_MapNodeState InState, bool InIsRequiredForTurn);

	UFUNCTION(BlueprintCallable)
	void SetState(EMetaGame_MapNodeState InState);

	UFUNCTION(BlueprintCallable)
	void UpdateNode();

	UFUNCTION(BlueprintCallable)
	void UpdateScale(float NewMultiplier);

	FName GetNodeID() const { return NodeDataPtr ? NodeDataPtr->ID : FName(); }

protected:

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int WidgetDefaultSize;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* WidgetComponent;

	UPROPERTY()
	class UMetaMapNodeWidget* NodeWidget;

	const FMetaGame_MapNodeData* NodeDataPtr = nullptr;
	
	EMetaGame_MapNodeState NodeState;
	bool IsRequiredForTurn;
};
