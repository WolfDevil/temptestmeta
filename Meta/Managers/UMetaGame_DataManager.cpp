// Fill out your copyright notice in the Description page of Project Settings.


#include "UMetaGame_DataManager.h"

#include "T01/Core/Settings/Meta/MetaGameSettings.h"

// HACK: check if type T has ID
template <typename T, typename = void>
struct THasID : std::false_type
{
};

template <typename T>
struct THasID<T, std::void_t<decltype(std::declval<T>().ID)>> : std::true_type
{
};


void UMetaGame_DataManager::Initialize()
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
	CachedTurnData.Empty();

	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->SquadPositionsDataTable, CachedSquadPositions, "SquadPositionsNodes");
	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->ActivitiesDataTable, CachedActivityNodes, "ActivityNodes");
	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->LoreNodesDataTable, CachedLoreNodes, "LoreNodes");
	CacheDataTableToMap<FMetaGame_MapNodeData>(Settings->MissionNodesDataTable, CachedMissionNodes, "MissionNodes");
	CacheDataTableToArray<FMetaGame_TurnData>(Settings->TurnsDataTable, CachedTurnData, "MissionNodes");
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
	return CachedTurnData.Num();
}

const FMetaGame_TurnData* UMetaGame_DataManager::GetTurnData(int Index) const
{
	if (Index >= CachedTurnData.Num()) return nullptr;
	return CachedTurnData[Index];
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
