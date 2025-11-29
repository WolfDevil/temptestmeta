// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UMetaGame_DataManager.generated.h"

struct FMetaGame_TurnData;
struct FMetaGame_MapNodeData;

UCLASS()
class T01_API UMetaGame_DataManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	const FMetaGame_MapNodeData* GetSquadPosition(FName ID) const;

	bool IsActivityNodesContainsID(FName ID) const;
	const FMetaGame_MapNodeData* GetActivityNode(FName ID) const;

	bool IsLoreNodesContainsID(FName ID) const;
	const FMetaGame_MapNodeData* GetLoreNode(FName ID) const;

	bool IsMissionNodesContainsID(FName ID) const;
	const FMetaGame_MapNodeData* GetMissionNode(FName ID) const;

	int GetTurnsCount() const;
	const FMetaGame_TurnData* GetTurnData(int Index) const;

private:
	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedTables;

	TMap<FName, const FMetaGame_MapNodeData*> CachedSquadPositions;
	TMap<FName, const FMetaGame_MapNodeData*> CachedActivityNodes;
	TMap<FName, const FMetaGame_MapNodeData*> CachedMissionNodes;
	TMap<FName, const FMetaGame_MapNodeData*> CachedLoreNodes;
	
	TArray<const FMetaGame_TurnData*> CachedTurnData;
	

	template <typename T>
	void CacheDataTableToMap(TSoftObjectPtr<UDataTable> TableAsset, TMap<FName, const T*>& OutCache, FString ContextInfo);

	template <typename T>
	void CacheDataTableToArray(TSoftObjectPtr<UDataTable> TableAsset, TArray<const T*>& OutCache, FString ContextInfo);
};
