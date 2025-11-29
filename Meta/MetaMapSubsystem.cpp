#include "MetaMapSubsystem.h"

#include "T01/Core/Gameplay/T01WorldSettings.h"
#include "T01/Core/Settings/Meta/MetaGameSettings.h"

DEFINE_STAT(STAT_UMetaMapSubsystem_Tick);

class UMetaGameSubsystem;

void UMetaMapSubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UMetaMapSubsystem_Tick);
	
	Super::Tick(DeltaTime);

	if (bIsTransitionMode)
	{
		if (SpawnedNodes.Num() == 1)
		{
			auto Node = SpawnedNodes[0];
			Node->SetActorLocation(TransitionCurve.Eval(TransitionTimer / 2.0f));
			TransitionTimer += DeltaTime;

			if (TransitionTimer >= 2.0f)
			{
				bIsTransitionMode = false;
				TransitionTimer = 0.0f;
				Node->SetActorLocation(TransitionCurve.Eval(1.0f));
				OnTransitionComplete.Broadcast();
				OnTransitionComplete.Clear();
				TransitionCurve.Reset();
			}
		}
	}
	else
	{
		//DrawConnections(NodeDatas, NodeStates, DeltaTime);
	}
}

void UMetaMapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "UMetaMapSubsystem::Initialize");
}

void UMetaMapSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UMetaMapSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!World) return false;

	
#if WITH_EDITOR
	if (!World->IsEditorWorld())
	{
		return false;
	}
#else
	if(!World->IsGameWorld())
	{
		return false;
	}
#endif


	const auto T01WorldSettings = Cast<AT01WorldSettings>(World->GetWorldSettings());
	if (!T01WorldSettings) return false;

	return T01WorldSettings->LevelTags.HasTagExact(FTags::Level_IsMetaMap);
}

bool UMetaMapSubsystem::IsInTransition()
{
	return bIsTransitionMode;
}

void UMetaMapSubsystem::SetupMap(TArray<const FMetaGame_MapNodeData*> InNodeData, TMap<FName, EMetaGame_MapNodeState> InNodeStates, TArray<FName> RequiredNodesForTurn)
{
	NodeDatas = InNodeData;
	NodeStates = InNodeStates;
	ClearMap();
	SpawnNodes(NodeDatas, NodeStates, RequiredNodesForTurn);
}

void UMetaMapSubsystem::StartTransition(const FMetaGame_MapNodeData* InSquadNodeData, FVector InEndLocation)
{
	ClearMap();

	TMap<FName, EMetaGame_MapNodeState> NodeState;
	NodeState.Add(InSquadNodeData->ID,  EMetaGame_MapNodeState::Unlocked );

	SpawnNodes(TArray<const FMetaGame_MapNodeData*>{InSquadNodeData}, NodeState, TArray<FName>());

	auto NewCurve = FInterpCurveVector();
	NewCurve.AddPoint(0.0f, InSquadNodeData->WorldPosition);
	NewCurve.AddPoint(1.0f, InEndLocation);

	if (InSquadNodeData->TransitionPoints.Num() > 0)
	{
		float Step = 1.0f / (InSquadNodeData->TransitionPoints.Num() + 1);
		for (int i = 0; i < InSquadNodeData->TransitionPoints.Num(); i++)
		{
			float T = Step * (i + 1);
			NewCurve.AddPoint(T, InSquadNodeData->TransitionPoints[i]);
		}
	}

	for (int i = 0; i < NewCurve.Points.Num(); i++)
	{
		NewCurve.Points[i].InterpMode = CIM_CurveAuto;
	}
	NewCurve.AutoSetTangents();

	this->TransitionCurve = NewCurve;
	this->TransitionTimer = 0.0f;
	bIsTransitionMode = true;
}

void UMetaMapSubsystem::UpdateNodeState(FName ID, EMetaGame_MapNodeState State)
{
	NodeStates[ID] = State;
}

EMetaGame_MapNodeState UMetaMapSubsystem::GetNodeState(FName ID)
{
	return NodeStates[ID];
}

UMetaGame_NodeAction_Base* UMetaMapSubsystem::GetNodeActionInstance(FName ID)
{
	UMetaGame_NodeAction_Base* NodeAction = NodeActionInstances.FindRef(ID);
	return NodeAction;
}

void UMetaMapSubsystem::SpawnNodes(TArray<const FMetaGame_MapNodeData*> InNodeData, TMap<FName, EMetaGame_MapNodeState> InNodeStates,
                                   TArray<FName> RequiredNodesForTurn)
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto Class = MetaGameSettings->NodeActorClass.Get();

	for (auto Node : InNodeData)
	{
		auto State = InNodeStates.Find(Node->ID);

		FVector Location = Node->WorldPosition;
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		auto Actor = GetWorld()->SpawnActor<AMetaGame_MapNodeActor>(Class, Location, Rotation, SpawnInfo);
		Actor->InitNode(*Node, *State, RequiredNodesForTurn.Contains(Node->ID));
		SpawnedNodes.Add(Actor);

		if (Node->NodeActionDataClass)
		{
			UMetaGame_NodeAction_Base* Instance = NewObject<UMetaGame_NodeAction_Base>(this, Node->NodeActionDataClass);
			if (Instance && Node->NodeActionDataAsset)
			{
				Instance->InitWithData(Node->ID, Node->NodeActionDataAsset);
			}

			NodeActionInstances.Add(Node->ID, Instance);
		}
	}
}

void UMetaMapSubsystem::ClearMap()
{
	for (auto Node : SpawnedNodes)
	{
		Node->Destroy();
	}
	SpawnedNodes.Empty();
}
