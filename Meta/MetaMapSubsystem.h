#pragma once
#include "Actors/AMetaGame_MapNodeActor.h"
#include "Data/Node/EMetaGame_MapNodeState.h"
#include "Data/Node/MetaGame_MapNodeData.h"
#include "T01/Utils/StatsGroups.h"
#include "MetaMapSubsystem.generated.h"

DECLARE_CYCLE_STAT_EXTERN(TEXT("UMetaMapSubsystem::Tick()::Overall"), STAT_UMetaMapSubsystem_Tick, STATGROUP_T01_Global, T01_API);

DECLARE_MULTICAST_DELEGATE(FOnTransitionComplete);

UCLASS()
class UMetaMapSubsystem : public UTickableWorldSubsystem
{
public:
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UMetaMapSubsystem, STATGROUP_Tickables)
	}

	virtual void Tick(float DeltaTime) override;

private:
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	FOnTransitionComplete OnTransitionComplete;

	bool IsInTransition();

	UFUNCTION(BlueprintCallable)
	void SetupMap(TArray<FMetaGame_MapNodeData> InNodeData, TMap<FName, EMetaGame_MapNodeState> InNodeStates, TArray<FName> RequiredNodesForTurn);

	UFUNCTION(BlueprintCallable)
	void StartTransition(FMetaGame_MapNodeData InSquadNodeData, FVector InEndLocation);

	UFUNCTION(BlueprintCallable)
	void UpdateNodeState(FName ID, EMetaGame_MapNodeState State);

	UFUNCTION(BlueprintCallable)
	EMetaGame_MapNodeState GetNodeState(FName ID);

	UFUNCTION(BlueprintCallable)
	UMetaGame_NodeAction_Base* GetNodeActionInstance(FName ID);

private:
	// void OnNodeClicked(FName ID);
	void SpawnNodes(TArray<FMetaGame_MapNodeData> InNodeData, TMap<FName, EMetaGame_MapNodeState> InNodeStates, TArray<FName> RequiredNodesForTurn);
	void DrawConnections(TArray<FMetaGame_MapNodeData> InNodeData, TMap<FName, EMetaGame_MapNodeState> InNodeStates, float DeltaTime);
	void ClearMap();

	TArray<FMetaGame_MapNodeData> NodeDatas;
	TMap<FName, EMetaGame_MapNodeState> NodeStates;

	UPROPERTY()
	TMap<FName, UMetaGame_NodeAction_Base*> NodeActionInstances;

	UPROPERTY()
	TArray<AMetaGame_MapNodeActor*> SpawnedNodes;

	bool bIsTransitionMode;
	float TransitionTimer;
	FInterpCurveVector TransitionCurve;
};
