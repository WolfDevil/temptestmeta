// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MetaGame_NodeAction_Base.generated.h"

class UMetaGame_NodeAction_BaseData;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class T01_API UMetaGame_NodeAction_Base : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FName NodeID;

	UPROPERTY(BlueprintReadOnly)
	UMetaGame_NodeAction_BaseData* DataAsset;

	virtual void InitWithData(const FName InNodeID, UMetaGame_NodeAction_BaseData* InData)
	{
		this->NodeID = InNodeID;
		DataAsset = InData;
	}

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteNodeAction(UObject* Context);
};
