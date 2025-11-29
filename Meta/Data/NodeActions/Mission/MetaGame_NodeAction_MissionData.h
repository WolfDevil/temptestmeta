// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_BaseData.h"

#include "MetaGame_NodeAction_MissionData.generated.h"

/**
 * 
 */
UCLASS()
class T01_API UMetaGame_NodeAction_MissionData : public UMetaGame_NodeAction_BaseData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Map;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ForcedFighters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Fighters"))
	int MaxFightersCount;
};
