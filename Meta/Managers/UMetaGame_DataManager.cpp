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

	CacheDataTable<FMetaGame_MapNodeData>(Settings->SquadPositionsDataTable, CachedSquadPositions, "SquadPositions");
}

const FMetaGame_MapNodeData* UMetaGame_DataManager::GetSquadPosition(FName ID) const
{
	if (ID.IsNone()) return nullptr;
	const FMetaGame_MapNodeData* const* Found = CachedSquadPositions.Find(ID);
	return Found ? *Found : nullptr;
}


template <typename T>
void UMetaGame_DataManager::CacheDataTable(TSoftObjectPtr<UDataTable> TableAsset, TMap<FName, const T*>& OutCache, FString ContextInfo)
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
