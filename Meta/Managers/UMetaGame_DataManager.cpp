// Fill out your copyright notice in the Description page of Project Settings.


#include "UMetaGame_DataManager.h"

#include "T01/Core/Settings/Meta/MetaGameSettings.h"
#include "T01/Core/Subsystem/Meta/Data/MetaGame_SkillData.h"

// HACK: check if type T has ID
template <typename T, typename = void>
struct THasID : std::false_type
{
};

template <typename T>
struct THasID<T, std::void_t<decltype(std::declval<T>().ID)>> : std::true_type
{
};


void UMetaGame_DataManager::Initialize() //TODO: Make async
{
	const UMetaGameSettings* Settings = GetDefault<UMetaGameSettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("MetaGame_DataManager: MetaGameSettings not found!"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("MetaGame_DataManager: Starting initialization..."));

	LoadedTables.Empty();
	CachedSquadPositions.Empty();
	CachedActivityNodes.Empty();
	CachedLoreNodes.Empty();
	CachedMissionNodes.Empty();
	CachedRewardsData.Empty();
	CachedThreatsData.Empty();
	CachedFightersData.Empty();
	CachedLoreData.Empty();
	CachedTutorialData.Empty();

	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->SquadPositionsDataTable, CachedSquadPositions, "SquadPositionsNodes");
	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->ActivitiesDataTable, CachedActivityNodes, "ActivityNodes");
	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->LoreNodesDataTable, CachedLoreNodes, "LoreNodes");
	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->MissionNodesDataTable, CachedMissionNodes, "MissionNodes");
	CacheDataTableToMap<FMetaGame_RewardData>(Settings->RewardsDataTable, CachedRewardsData, "RewardsData");
	CacheDataTableToMap<FMetaGame_ThreatData>(Settings->ThreatsDataTable, CachedThreatsData, "RewardsData");
	CacheDataTableToMap<FMetaGame_FighterData>(Settings->FightersDataTable, CachedFightersData, "RewardsData");
	CacheDataTableToMap<FMetaGame_LoreData>(Settings->LoreDataTable, CachedLoreData, "LoreData");
	CacheDataTableToMap<FMetaGame_TutorialData>(Settings->LoreDataTable, CachedTutorialData, "LoreData");

	CachedTurnsData.Empty();
	CachedSkillsData.Empty();
	CacheDataTableToArray<FMetaGame_TurnData>(Settings->TurnsDataTable, CachedTurnsData, "TurnData");
	CacheDataTableToArray<FMetaGame_SkillData>(Settings->SkillsDataTable, CachedSkillsData, "TurnData");

	CachedDialoguesDataTable = Settings->DialoguesDataTable.LoadSynchronous();
}

const FMetaGame_MapNodeData* UMetaGame_DataManager::GetSquadPosition(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_MapNodeData* const* Found = CachedSquadPositions.Find(ID);
	return Found ? *Found : nullptr;
}

bool UMetaGame_DataManager::IsActivityNodesContainsID(FName ID) const
{
	if (ID.IsNone()) return false;
	return CachedActivityNodes.Contains(ID);
}

const FMetaGame_MapNodeData* UMetaGame_DataManager::GetActivityNode(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_MapNodeData* const* Found = CachedActivityNodes.Find(ID);
	return Found ? *Found : nullptr;
}

bool UMetaGame_DataManager::IsLoreNodesContainsID(FName ID) const
{
	if (ID.IsNone()) return false;
	return CachedLoreNodes.Contains(ID);
}

const FMetaGame_MapNodeData* UMetaGame_DataManager::GetLoreNode(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_MapNodeData* const* Found = CachedLoreNodes.Find(ID);
	return Found ? *Found : nullptr;
}

bool UMetaGame_DataManager::IsMissionNodesContainsID(FName ID) const
{
	if (ID.IsNone()) return false;
	return CachedMissionNodes.Contains(ID);
}


const FMetaGame_MapNodeData* UMetaGame_DataManager::GetMissionNode(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_MapNodeData* const* Found = CachedMissionNodes.Find(ID);
	return Found ? *Found : nullptr;
}

int UMetaGame_DataManager::GetTurnsCount() const
{
	return CachedTurnsData.Num();
}

const FMetaGame_TurnData* UMetaGame_DataManager::GetTurnData(int Index) const
{
	if (Index >= CachedTurnsData.Num()) return nullptr;
	return CachedTurnsData[Index];
}

const FMetaGame_RewardData* UMetaGame_DataManager::GetRewardData(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_RewardData* const* Found = CachedRewardsData.Find(ID);
	return Found ? *Found : nullptr;
}

const FMetaGame_ThreatData* UMetaGame_DataManager::GetThreatData(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_ThreatData* const* Found = CachedThreatsData.Find(ID);
	return Found ? *Found : nullptr;
}

const FMetaGame_FighterData* UMetaGame_DataManager::GetFighterData(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_FighterData* const* Found = CachedFightersData.Find(ID);
	return Found ? *Found : nullptr;
}

const FMetaGame_SkillData* UMetaGame_DataManager::GetSkill(FName ID, int32 Level) const
{
	const FMetaGame_SkillData* const* Found = CachedSkillsData.FindByPredicate(
		[ID, Level](const FMetaGame_SkillData* Item)
		{
			return Item && Item->ID == ID && Item->Level == Level;
		}
	);

	return Found ? *Found : nullptr;
}

const TArray<const FMetaGame_SkillData*>& UMetaGame_DataManager::GetAllSkills() const
{
	return CachedSkillsData;
}

const FMetaGame_LoreData* UMetaGame_DataManager::GetLoreData(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_LoreData* const* Found = CachedLoreData.Find(ID);
	return Found ? *Found : nullptr;
}

const FMetaGame_TutorialData* UMetaGame_DataManager::GetTutorialData(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_TutorialData* const* Found = CachedTutorialData.Find(ID);
	return Found ? *Found : nullptr;
}

UDataTable* UMetaGame_DataManager::GetDialoguesDataTable() const
{
	return CachedDialoguesDataTable;
}


template <typename T>
void UMetaGame_DataManager::CacheDataTableToMap(TSoftObjectPtr<UDataTable> TableAsset, TMap<FName, const T*>& OutCache, FString ContextInfo)
{
	UDataTable* DataTable = TableAsset.LoadSynchronous();

	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("MetaDataManager: Failed to load DataTable for %s"), *ContextInfo);
		return;
	}

	LoadedTables.Add(DataTable);

	for (auto It = DataTable->GetRowMap().CreateConstIterator(); It; ++It)
	{
		FName RowName = It.Key();

		if (T* RowData = reinterpret_cast<T*>(It.Value()))
		{
			if constexpr (THasID<T>::value)
			{
				OutCache.Add(RowData->ID, RowData);
			}
			else
			{
				OutCache.Add(RowName, RowData);
			}
		}
	}
}

template <typename T>
void UMetaGame_DataManager::CacheDataTableToArray(TSoftObjectPtr<UDataTable> TableAsset, TArray<const T*>& OutCache, FString ContextInfo)
{
	UDataTable* DataTable = TableAsset.LoadSynchronous();

	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("MetaDataManager: Failed to load DataTable for %s"), *ContextInfo);
		return;
	}

	LoadedTables.Add(DataTable);

	TArray<FName> RowNames = DataTable->GetRowNames();
	RowNames.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	OutCache.Reserve(RowNames.Num());

	for (const FName& RowName : RowNames)
	{
		if (uint8* RowDataPtr = DataTable->GetRowMap().FindRef(RowName))
		{
			const T* Data = reinterpret_cast<const T*>(RowDataPtr);
			OutCache.Add(Data);
		}
	}
}
