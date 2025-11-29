// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UMetaGame_DataManager.generated.h"

struct FMetaGame_MapNodeData;

UCLASS()
class T01_API UMetaGame_DataManager : public UObject
{
	GENERATED_BODY()

public:

	void Initialize();

	const FMetaGame_MapNodeData* GetSquadPosition(FName ID) const;
	

private:

	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedTables;

	TMap<FName, const FMetaGame_MapNodeData*> CachedSquadPositions;

	template <typename T>
	void CacheDataTable(TSoftObjectPtr<UDataTable> TableAsset, TMap<FName, const T*>& OutCache, FString ContextInfo);
};


