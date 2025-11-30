// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UMetaGame_DataManager.generated.h"

struct FMetaGame_TutorialData;
struct FMetaGame_LoreData;
struct FMetaGame_SkillData;
struct FMetaGame_FighterData;
struct FMetaGame_ThreatData;
struct FMetaGame_RewardData;
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

	const FMetaGame_RewardData* GetRewardData(FName ID) const;
	const TMap<FName, const FMetaGame_RewardData*>& GetCachedRewards() const { return CachedRewardsData; }

	const FMetaGame_ThreatData* GetThreatData(FName ID) const;
	const TMap<FName, const FMetaGame_ThreatData*>& GetCachedThreats() const { return CachedThreatsData; }

	const FMetaGame_FighterData* GetFighterData(FName ID) const;
	const TMap<FName, const FMetaGame_FighterData*>& GetCachedFighters() const { return CachedFightersData; }

	const FMetaGame_SkillData* GetSkill(FName ID, int32 Level) const;
	const TArray<const FMetaGame_SkillData*>& GetAllSkills() const;

	const FMetaGame_LoreData* GetLoreData(FName ID) const;
	
	const FMetaGame_TutorialData* GetTutorialData(FName ID) const;

	UDataTable* GetDialoguesDataTable() const;

private:
	UPROPERTY()
	TArray<TObjectPtr<UDataTable>> LoadedTables;

	UPROPERTY()
	TObjectPtr<UDataTable> CachedDialoguesDataTable;

	TMap<FName, const FMetaGame_MapNodeData*> CachedSquadPositions;
	TMap<FName, const FMetaGame_MapNodeData*> CachedActivityNodes;
	TMap<FName, const FMetaGame_MapNodeData*> CachedMissionNodes;
	TMap<FName, const FMetaGame_MapNodeData*> CachedLoreNodes;
	TMap<FName, const FMetaGame_RewardData*> CachedRewardsData;
	TMap<FName, const FMetaGame_ThreatData*> CachedThreatsData;
	TMap<FName, const FMetaGame_FighterData*> CachedFightersData;
	TMap<FName, const FMetaGame_LoreData*> CachedLoreData;
	TMap<FName, const FMetaGame_TutorialData*> CachedTutorialData;

	TArray<const FMetaGame_TurnData*> CachedTurnsData;
	TArray<const FMetaGame_SkillData*> CachedSkillsData;


	template <typename T>
	void CacheDataTableToMap(TSoftObjectPtr<UDataTable> TableAsset, TMap<FName, const T*>& OutCache, FString ContextInfo);

	template <typename T>
	void CacheDataTableToArray(TSoftObjectPtr<UDataTable> TableAsset, TArray<const T*>& OutCache, FString ContextInfo);
};
