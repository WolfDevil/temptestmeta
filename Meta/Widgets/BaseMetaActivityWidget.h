// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommonUserWidget.h"
#include "T01/Core/Subsystem/Meta/Data/MetaGame_FighterData.h"
#include "T01/Core/Subsystem/Meta/Data/Node/MetaGame_MapNodeData.h"

#include "BaseMetaActivityWidget.generated.h"

UCLASS()
class T01_API UBaseMetaActivityWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
#pragma warning(push)
#pragma warning(disable: 4263 4264) // intentionally function with same name, but different goal
	UFUNCTION(BlueprintNativeEvent)
	void Initialize(UMetaGame_NodeAction_Base* InNodeAction);
#pragma warning(pop)

	UFUNCTION(BlueprintNativeEvent)
	void InitializeWithFighters(UMetaGame_NodeAction_Base* InNodeAction, const TArray<FMetaGame_FighterData>& Fighters);

protected:
	UPROPERTY(BlueprintReadWrite)
	UMetaGame_NodeAction_Base* NodeAction;

#pragma warning(push)
#pragma warning(disable: 4263 4264) // intentionally function with same name, but different goal
};
#pragma warning(pop)