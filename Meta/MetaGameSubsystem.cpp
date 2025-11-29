#include "MetaGameSubsystem.h"

#include "Data/EMetaGame_NextTurnStatus.h"
#include "Data/NodeActions/Activity/MetaGame_NodeAction_Activity.h"
#include "Data/NodeActions/Activity/MetaGame_NodeAction_ActivityData.h"
#include "Data/NodeActions/Base/MetaGame_NodeAction_BaseData.h"
#include "Data/NodeActions/Lore/MetaGame_LoreData.h"
#include "Data/NodeActions/Lore/MetaGame_NodeAction_Lore.h"
#include "Data/NodeActions/Mission/MetaGame_NodeAction_Mission.h"
#include "Data/NodeActions/Mission/MetaGame_NodeAction_MissionData.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/UMetaGame_DataManager.h"
#include "T01/Core/Gameplay/T01GameInstance.h"
#include "T01/Core/Settings/Meta/MetaGameSettings.h"
#include "T01/Core/Subsystem/Savegame/T01Save.h"
#include "T01/Core/Subsystem/Savegame/T01SaveGameSubsystem.h"
#include "T01/Utils/UIUtils.h"
#include "Widgets/BaseMetaLoreWidget.h"


bool UMetaGameSubsystem::CanBeSaved(FString& FailureReason) const
{
	return true;
}

void UMetaGameSubsystem::Save(UT01Save* SaveObject, void* ParentObject)
{
	StorageInventory->Save();

	SaveObject->Progression.SaveGUID = FGuid::NewGuid();
	SaveObject->Progression.CurrentTurnIndex = CurrentTurnIndex;
	SaveObject->Progression.Resources = Resources;
	SaveObject->Progression.CompletedLoreIDs = CompletedLoreIDs;
	SaveObject->Progression.LockedUnitsForOneTurn = LockedUnitsForOneTurn;
	SaveObject->Progression.RequestedNextTurn = RequestedNextTurn;

	SaveObject->Progression.NodeStates.Empty();
	for (auto NodeState : NodeStates)
	{
		uint8 State = static_cast<uint8>(NodeState.Value);
		SaveObject->Progression.NodeStates.Add(NodeState.Key, State);
	}

	SaveObject->Progression.ActivitiesToResolve.Empty();
	for (auto PreparedActivity : ActionsToResolve)
	{
		FMetaGame_PreparedActivityData_Save PreparedActivitySave = FMetaGame_PreparedActivityData_Save();
		PreparedActivitySave.ID = PreparedActivity.ID;
		for (auto FighterData : PreparedActivity.FightersData)
		{
			PreparedActivitySave.AssignedFightersID.Add(FighterData.ID);
		}
		PreparedActivitySave.Scheduled = PreparedActivity.Scheduled;
		SaveObject->Progression.ActivitiesToResolve.Add(PreparedActivitySave);
	}

	SaveObject->Progression.GottenRewards.Empty();
	for (auto Reward : GottenRewards)
	{
		FMetaGame_RewardNotificationData_Save RewardSave = FMetaGame_RewardNotificationData_Save();
		RewardSave.ActivityID = Reward.ActivityID;
		RewardSave.GUID = Reward.GUID;
		RewardSave.GetOnTurnNum = Reward.GetOnTurnNum;
		RewardSave.NotificationGetTime = Reward.NotificationGetTime;

		for (auto RewardData : Reward.Rewards)
		{
			RewardSave.Rewards.Add(RewardData.Key, RewardData.Value);
		}
		SaveObject->Progression.GottenRewards.Add(RewardSave);
	}
}

void UMetaGameSubsystem::Load(UT01Save* SaveObject, void* ParentObject)
{
	return;
}

void UMetaGameSubsystem::SaveMeta()
{
	const auto Settings = GetDefault<UMetaGameSettings>();
	if (!Settings->EnableSaveFunctionality) return;
	UT01SavegameSubsystem::SaveGame(this, UT01SavegameSubsystem::GetNextSaveSlotNameBy(ESaveType::QuickSave));
}

bool UMetaGameSubsystem::IsInitialized() const
{
	return bIsInitialized;
}

void UMetaGameSubsystem::OnMetaGameModeLoaded()
{
	UpdateMap();
	if (RequestedNextTurn) NextTurnOnStart();
	else
	{
		CheckTurnLore();
		CheckTurnDialogue();
		CheckTurnTutorial();
	}
}

void UMetaGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	//Modules
	DataManager = NewObject<UMetaGame_DataManager>(this, TEXT("MetaDataManager"));

	DataManager->Initialize();


	//Other
	Collection.InitializeDependency<UPoolSubsystem>();
	MetaMapSubsystem = Collection.InitializeDependency<UMetaMapSubsystem>();

	AllFighters.Empty();
	AllFighters = GetAllFightersData();

	const auto Settings = GetDefault<UMetaGameSettings>();

	{
		auto StorageInventoryClass = Settings->StorageInventoryHolderClass.Get();
		FVector Location = FVector::ZeroVector;
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		StorageInventory = GetWorld()->SpawnActor<AMetaGame_StorageInventory>(StorageInventoryClass, Location, Rotation, SpawnInfo);
		StorageInventory->Initialize(TEXT("MetaStorage"));
	}

	auto ProgressionSave = UT01SavegameSubsystem::GetProgression();
	if (ProgressionSave.SaveGUID.IsValid() && Settings->EnableSaveFunctionality)
	{
		ApplyMetaProgressionSave(ProgressionSave);
	}
	else
	{
		RequestedNextTurn = false;
		Resources.Empty();
		for (auto Reward : GetAllRewardsData())
		{
			if (Reward.Type == EMetaGame_RewardResourceType::Resource)
			{
				Resources.Add(Reward.ID, 200);
			}
		}
	}

	auto RangerInventoryClass = Settings->RangerInventoryHolderClass.Get();
	RangerInventories.Empty();
	for (auto Fighter : AllFighters)
	{
		FVector Location = FVector::ZeroVector;
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		auto Actor = GetWorld()->SpawnActor<AMetaGame_RangerInventory>(RangerInventoryClass, Location, Rotation, SpawnInfo);
		Actor->Initialize(Fighter.ID.ToString());
		RangerInventories.Add(Fighter.ID, Actor);
	}


	bIsInitialized = true;
}

void UMetaGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UMetaGameSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!World) return false;

	const auto T01WorldSettings = Cast<AT01WorldSettings>(World->GetWorldSettings());
	if (!T01WorldSettings) return false;

	return T01WorldSettings->LevelTags.HasTagExact(FTags::Level_IsMetaMap) && World->IsGameWorld();
}

void UMetaGameSubsystem::ApplyMetaProgressionSave(FT01SaveModule_MetaProgression Progression)
{
	StorageInventory->Load();

	CurrentTurnIndex = Progression.CurrentTurnIndex;
	Resources = Progression.Resources;
	CompletedLoreIDs = Progression.CompletedLoreIDs;
	LockedUnitsForOneTurn = Progression.LockedUnitsForOneTurn;
	RequestedNextTurn = Progression.RequestedNextTurn;

	NodeStates.Empty();
	for (auto NodeStatePair : Progression.NodeStates)
	{
		auto Name = NodeStatePair.Key;
		auto State = static_cast<EMetaGame_MapNodeState>(NodeStatePair.Value);
		NodeStates.Add(Name, State);
	}

	for (auto [ID, AssignedFightersID, Scheduled] : Progression.ActivitiesToResolve)
	{
		auto Data = FMetaGame_PreparedActionData();
		Data.Scheduled = Scheduled;
		Data.ID = ID;
		Data.FightersData.Empty();
		for (const auto Fighter : AssignedFightersID)
		{
			Data.FightersData.Add(GetFighterData(Fighter));
		}
		ActionsToResolve.Add(Data);
	}

	for (auto GottenRewardSave : Progression.GottenRewards)
	{
		auto Data = FMetaGame_RewardNotificationData();
		Data.GUID = GottenRewardSave.GUID;
		Data.ActivityID = GottenRewardSave.ActivityID;
		Data.GetOnTurnNum = GottenRewardSave.GetOnTurnNum;
		Data.NotificationGetTime = GottenRewardSave.NotificationGetTime;
		Data.Rewards = GottenRewardSave.Rewards;
	}
}

bool UMetaGameSubsystem::CanControlCamera()
{
	return ShowedActivityView == nullptr && ShowedLoreView == nullptr && ShowedInventoryView == nullptr && ShowedTutorialView == nullptr && ShowedDialogueView == nullptr;
}

bool UMetaGameSubsystem::CanShowHUD()
{
	return ShowedActivityView == nullptr && ShowedLoreView == nullptr && !MetaMapSubsystem->IsInTransition() && ShowedTutorialView == nullptr && ShowedDialogueView == nullptr;
}

bool UMetaGameSubsystem::CanShowFightersPanel()
{
	return ShowedLoreView == nullptr && !MetaMapSubsystem->IsInTransition() && ShowedDialogueView == nullptr;
}

TArray<FMetaGame_FighterData> UMetaGameSubsystem::GetAvailableFighters(FName ActivityID)
{
	TArray<FMetaGame_FighterData> Fighters;
	Fighters = GetTurnFighters();
	TArray<FMetaGame_FighterData> FightersToExclude;

	for (auto Fighter : LockedUnitsForOneTurn)
	{
		FName FighterNameCopy = FName(Fighter);
		auto FoundFighter = Fighters.FindByPredicate([FighterNameCopy](const FMetaGame_FighterData& FighterData)
		{
			return FighterData.ID == FighterNameCopy;
		});
		if (FoundFighter != nullptr)
		{
			FightersToExclude.AddUnique(*FoundFighter);
		}
	}

	for (auto Activity : ActionsToResolve)
	{
		if (ActivityID == Activity.ID) continue;
		for (auto FighterData : Activity.FightersData)
		{
			FightersToExclude.AddUnique(FighterData);
		}
	}

	for (auto Fighter : FightersToExclude)
	{
		Fighters.Remove(Fighter);
	}

	return Fighters;
}

TArray<FMetaGame_FighterData> UMetaGameSubsystem::GetTurnFighters()
{
	auto AllFightersData = GetAllFightersData();
	auto Turns = GetAllTurns();
	auto CurrentTurnData = Turns[CurrentTurnIndex];
	auto FightersIDs = CurrentTurnData.AvailableFighterIDs;

	TArray<FMetaGame_FighterData> Fighters;
	for (auto FighterData : AllFightersData)
	{
		if (FightersIDs.Contains(FighterData.ID))
		{
			Fighters.AddUnique(FighterData);
		}
	}

	return Fighters;
}

TArray<FMetaGame_FighterData> UMetaGameSubsystem::GetFightersOnActivity(FName ActivityID) const
{
	const auto NodeToResolveData = ActionsToResolve.FindByPredicate([ActivityID](const FMetaGame_PreparedActionData& ResolveData)
		{
			return ResolveData.ID.IsEqual(ActivityID);
		}
	);

	if (NodeToResolveData != nullptr)
	{
		const auto Fighters = NodeToResolveData->FightersData;
		return Fighters;
	}

	return TArray<FMetaGame_FighterData>();
}

FMetaGame_MapNodeData UMetaGameSubsystem::GetActivityThatOccupiedFighter(FName FighterID) const
{
	auto PreparedActivity = ActionsToResolve.FindByPredicate([FighterID](const FMetaGame_PreparedActionData& ResolveData)
	{
		return ResolveData.FightersData.ContainsByPredicate([FighterID](const FMetaGame_FighterData& Data)
		{
			return FighterID == Data.ID;
		});
	});
	if (PreparedActivity == nullptr) return FMetaGame_MapNodeData();
	return GetNodeData(PreparedActivity->ID);
}

TArray<FMetaGame_PreparedActionData> UMetaGameSubsystem::GetPendingActivities()
{
	return ActionsToResolve;
}

TArray<FMetaGame_RewardNotificationData> UMetaGameSubsystem::GetGottenRewards()
{
	return GottenRewards;
}

bool UMetaGameSubsystem::RemoveGottenRewardNotification(FGuid GUID)
{
	const auto FoundDataIndex = GottenRewards.IndexOfByPredicate([GUID](const FMetaGame_RewardNotificationData& Data)
	{
		return Data.GUID == GUID;
	});
	if (FoundDataIndex > -1)
	{
		GottenRewards.RemoveAt(FoundDataIndex);
		GottenRewards.Sort([](const FMetaGame_RewardNotificationData& Data1, const FMetaGame_RewardNotificationData& Data2)
		{
			return Data1.NotificationGetTime < Data2.NotificationGetTime;
		});
		OnGottenRewardsUpdated.Broadcast();
		OnGottenRewardsUpdatedDynamic.Broadcast();
		return true;
	}
	return false;
}

void UMetaGameSubsystem::RemoveGottenRewardNotificationFromLastTurn()
{
	TArray<FGuid> GUIDsToRemove;
	GUIDsToRemove.Empty();

	for (auto RewardNotification : GottenRewards)
	{
		if (RewardNotification.GetOnTurnNum <= CurrentTurnIndex - 1)
		{
			GUIDsToRemove.Add(RewardNotification.GUID);
		}
	}

	for (auto GUID : GUIDsToRemove)
	{
		int Idx = GottenRewards.IndexOfByPredicate([GUID](const FMetaGame_RewardNotificationData& Data)
		{
			return Data.GUID == GUID;
		});

		if (Idx > -1)
		{
			GottenRewards.RemoveAt(Idx);
		}
	}

	GottenRewards.Sort([](const FMetaGame_RewardNotificationData& Data1, const FMetaGame_RewardNotificationData& Data2)
	{
		return Data1.NotificationGetTime < Data2.NotificationGetTime;
	});

	OnGottenRewardsUpdated.Broadcast();
	OnGottenRewardsUpdatedDynamic.Broadcast();
}

void UMetaGameSubsystem::UpdateMap()
{
	ensure(DataManager);

	if (MetaMapSubsystem == nullptr) return;

	auto Turns = GetAllTurns();

	// auto Activities = GetAllActivities();
	// Activities.Append(GetAllLoreNodes());
	// Activities.Append(GetAllMissionNodes());


	if (Turns.Num() - 1 < CurrentTurnIndex) return;
	FMetaGame_TurnData CurrentTurnData = Turns[CurrentTurnIndex];


	TArray<const FMetaGame_MapNodeData*> MapNodes;
	FName SquadPosNodeID = CurrentTurnData.SquadPositionNodeID;


	const FMetaGame_MapNodeData* SquadNode = DataManager->GetSquadPosition(SquadPosNodeID);
	if (SquadNode == nullptr) return;

	MapNodes.Add(SquadNode);


	if (SquadNode->bShowHead)
	{
		if (Turns.Num() - 1 >= CurrentTurnIndex + 1)
		{
			FMetaGame_TurnData HeadTurnData = Turns[CurrentTurnIndex + 1];

			FName SquadPosID = HeadTurnData.SquadPositionNodeID;
			const FMetaGame_MapNodeData* HeadSquadNode = DataManager->GetSquadPosition(SquadPosID);
			if (HeadSquadNode == nullptr) return;
			MapNodes.Add(HeadSquadNode);
		}
	}


	if (SquadNode->bShowTail)
	{
		int TurnIndex = CurrentTurnIndex - 1;

		while (TurnIndex >= 0)
		{
			if (Turns.Num() - 1 >= TurnIndex)
			{
				FMetaGame_TurnData TailTurnData = Turns[TurnIndex];
				FName SquadPosID = TailTurnData.SquadPositionNodeID;
				const FMetaGame_MapNodeData* TailHeadSquadNode = DataManager->GetSquadPosition(SquadPosID);
				if (TailHeadSquadNode == nullptr) return;
				MapNodes.Add(TailHeadSquadNode);
			}

			TurnIndex--;
		}
	}

	if (DataManager != nullptr)
	{
		for (auto NodeID : CurrentTurnData.AvailableNodeIDs)
		{
			auto ActivityNodeData = DataManager->GetActivityNode(NodeID);
			if (ActivityNodeData != nullptr)
			{
				MapNodes.Add(ActivityNodeData);
				continue;
			}

			auto MissionNodeData = DataManager->GetMissionNode(NodeID);
			if (MissionNodeData != nullptr)
			{
				MapNodes.Add(MissionNodeData);
				continue;
			}

			auto LoreNodeData = DataManager->GetLoreNode(NodeID);
			if (LoreNodeData != nullptr)
			{
				MapNodes.Add(LoreNodeData);
			}
		}
	}


	for (auto Node : MapNodes)
	{
		auto NewState = EMetaGame_MapNodeState::Unlocked;
		if (Node->RequiredNodeIDs.Num() > 0)
		{
			for (auto ReqID : Node->RequiredNodeIDs)
			{
				if (NodeStates[ReqID] != EMetaGame_MapNodeState::Completed)
				{
					NewState = EMetaGame_MapNodeState::Locked;
					break;
				}
			}
		}

		if (NodeStates.Contains(Node->ID))
		{
			if (NodeStates[Node->ID] == EMetaGame_MapNodeState::Locked || NodeStates[Node->ID] == EMetaGame_MapNodeState::Unlocked)
			{
				NodeStates[Node->ID] = NewState;
			}
		}
		else
		{
			NodeStates.Add(Node->ID, NewState);
		}
	}

	MetaMapSubsystem->SetupMap(MapNodes, NodeStates, CurrentTurnData.RequiredNodeIDs);
}

void UMetaGameSubsystem::CheckTurnLore()
{
	auto Turns = GetAllTurns();

	if (Turns.Num() - 1 < CurrentTurnIndex) return;
	FMetaGame_TurnData CurrentTurnData = Turns[CurrentTurnIndex];
	FName LoreID = CurrentTurnData.StartLoreID;
	if (LoreID.IsNone()) return;
	if (CompletedLoreIDs.Contains(LoreID)) return;

	bool Success;
	ShowLoreUI(LoreID, Success);
}

void UMetaGameSubsystem::CheckTurnTutorial()
{
	auto Turns = GetAllTurns();

	if (Turns.Num() - 1 < CurrentTurnIndex) return;
	FMetaGame_TurnData CurrentTurnData = Turns[CurrentTurnIndex];
	FName TutorialID = CurrentTurnData.TutorialID;
	if (TutorialID.IsNone()) return;

	ShowTutorialWidget(TutorialID);
}

void UMetaGameSubsystem::CheckTurnDialogue()
{
	auto Turns = GetAllTurns();

	if (Turns.Num() - 1 < CurrentTurnIndex) return;
	FMetaGame_TurnData CurrentTurnData = Turns[CurrentTurnIndex];
	FName DialogueID = CurrentTurnData.DialogueID;
	if (DialogueID.IsNone()) return;

	ShowDialogue(DialogueID);
}

void UMetaGameSubsystem::ShowActivityUI(FName ID, bool& Success)
{
	Success = false;

	if (ShowedActivityView != nullptr) return;

	auto NodeData = GetNodeData(ID);
	const auto NodeActionInstance = MetaMapSubsystem->GetNodeActionInstance(ID);
	if (NodeActionInstance == nullptr) return;
	if (NodeData.NodeType != EMetaGame_MapNodeType::Activity) return;
	const auto NodeActivityInstance = Cast<UMetaGame_NodeAction_Activity>(NodeActionInstance);
	if (NodeActivityInstance == nullptr) return;

	if (NodeStates[ID] == EMetaGame_MapNodeState::Completed) return;


	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->ActivityViewWidget.LoadSynchronous();
	UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);

	if (NodeData.NodeActionDataAsset == nullptr
		|| !WidgetClass
		|| !World
		|| !PC
	)
	{
		return;
	}


	UBaseMetaActivityWidget* Widget = CreateWidget<UBaseMetaActivityWidget>(PC, WidgetClass);
	if (!Widget)
	{
		Success = false;
		return;
	}


	auto NodeToResolveData = ActionsToResolve.FindByPredicate([NodeData](const FMetaGame_PreparedActionData& ResolveData)
		{
			return ResolveData.ID.IsEqual(NodeData.ID);
		}
	);

	if (NodeToResolveData != nullptr)
	{
		const auto Fighters = NodeToResolveData->FightersData;
		Widget->InitializeWithFighters(NodeActivityInstance, Fighters);
	}
	else
	{
		Widget->Initialize(NodeActivityInstance);
	}

	Widget->AddToViewport(100);
	ShowedActivityView = Widget;
	Success = true;

	OnOpenedWindow.Broadcast();
	OnOpenedWindowDynamic.Broadcast();
}

void UMetaGameSubsystem::ShowLoreUI(FName LoreID, bool& Success)
{
	Success = false;

	if (ShowedLoreView != nullptr) return;
	if (CompletedLoreIDs.Contains(LoreID)) return;

	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->LoreViewWidget.LoadSynchronous();
	UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);

	if (!WidgetClass
		|| !World
		|| !PC)
	{
		Success = false;
		return;
	}

	auto LoreData = GetLoreData(LoreID);
	if (LoreData.ID != LoreID)
	{
		Success = false;
		return;
	}

	UBaseMetaLoreWidget* Widget = CreateWidget<UBaseMetaLoreWidget>(PC, WidgetClass);
	if (!Widget)
	{
		Success = false;
		return;
	}

	Widget->Initialize(LoreData);
	Widget->AddToViewport(100);
	ShowedLoreView = Widget;
	Success = true;

	OnOpenedWindow.Broadcast();
	OnOpenedWindowDynamic.Broadcast();
}

void UMetaGameSubsystem::ShowLoreUIFromNode(FName NodeID, bool& Success)
{
	Success = false;

	auto NodeData = GetNodeData(NodeID);
	const auto NodeActionInstance = MetaMapSubsystem->GetNodeActionInstance(NodeID);
	if (NodeActionInstance == nullptr) return;
	if (NodeData.NodeType != EMetaGame_MapNodeType::Activity) return;
	const auto NodeLoreInstance = Cast<UMetaGame_NodeAction_Lore>(NodeActionInstance);
	if (NodeLoreInstance == nullptr) return;
	const auto NodeLoreData = Cast<UMetaGame_NodeAction_LoreData>(NodeLoreInstance->DataAsset);
	if (NodeLoreData == nullptr) return;

	ShowLoreUI(NodeLoreData->LoreID, Success);
}

void UMetaGameSubsystem::ShowRewardsWindow(FMetaGame_RewardNotificationData Data)
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->RewardsWindowWidget.LoadSynchronous();
	UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);

	if (!WidgetClass || !World || !PC) return;

	UBaseMetaActivityRewardsWidget* Widget = CreateWidget<UBaseMetaActivityRewardsWidget>(PC, WidgetClass);
	if (!Widget) return;

	Widget->AddToViewport(100);
	Widget->Initialize(Data);
}

void UMetaGameSubsystem::ShowMissionNotAssignedUI()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->MissionNotAssignedWindow.LoadSynchronous();
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!WidgetClass || !PC) return;

	UCommonUserWidget* Widget = CreateWidget<UCommonUserWidget>(PC, WidgetClass);
	if (!Widget) return;


	Widget->AddToViewport(100);
}

void UMetaGameSubsystem::ShowActivitiesNotAssignedUI(bool bCanIgnore)
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->ActivityNotAssignedWindow.LoadSynchronous();
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!WidgetClass || !PC) return;

	UBaseMetaActivityNotAssignedWindow* Widget = CreateWidget<UBaseMetaActivityNotAssignedWindow>(PC, WidgetClass);
	if (!Widget) return;
	Widget->AddToViewport(100);
	Widget->Setup(bCanIgnore);
}

void UMetaGameSubsystem::ShowActivityCantBeResolved()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->ActivityCantBeResolved.LoadSynchronous();
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!WidgetClass || !PC) return;

	UCommonUserWidget* Widget = CreateWidget<UCommonUserWidget>(PC, WidgetClass);
	if (!Widget) return;


	Widget->AddToViewport(100);
}

void UMetaGameSubsystem::OpenFighterInventory(FName FighterID)
{
	if (!RangerInventories.Contains(FighterID))
		return;

	auto InventoryActor = RangerInventories[FighterID];
	if (InventoryActor == nullptr)
		return;

	if (ShowedInventoryView != nullptr)
	{
		CloseFighterInventory();
	}

	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->InventoryWidget.LoadSynchronous();
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!WidgetClass || !PC)
		return;

	const auto CoreUI = UIUtils::GetCoreUI(GetWorld());
	if (!CoreUI)
		return;

	const auto Widget = CoreUI->PushWidget(WidgetClass);
	if (const auto MetaInventoryWidget = Cast<UBaseMetaInventoryWidget>(Widget))
	{
		MetaInventoryWidget->InitializeWindow(FighterID, InventoryActor->InventoryComponent, StorageInventory->InventoryComponent);
		ShowedInventoryView = MetaInventoryWidget;

		OnOpenedWindow.Broadcast();
		OnOpenedWindowDynamic.Broadcast();
	}
}

void UMetaGameSubsystem::OpenFirstFighterInventory()
{
	auto Fighters = GetTurnFighters();
	if (Fighters.Num() > 0)
	{
		OpenFighterInventory(Fighters[0].ID);
	}
}

void UMetaGameSubsystem::CloseFighterInventory()
{
	if (ShowedInventoryView != nullptr)
	{
		ShowedInventoryView->DeactivateWidget();
		// ShowedInventoryView->RemoveFromParent();
		ShowedInventoryView = nullptr;

		OnClosedWindow.Broadcast();
		OnClosedWindowDynamic.Broadcast();
	}
}

void UMetaGameSubsystem::ShowTutorialWidget(FName TutorialID)
{
	auto TutorialData = GetTutorialData(TutorialID);
	if (TutorialData.Widget == nullptr) return;
	UClass* WidgetClass = TutorialData.Widget.LoadSynchronous();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!WidgetClass || !PC)
		return;

	if (UBaseMetaTutorialWidget* Widget = CreateWidget<UBaseMetaTutorialWidget>(PC, WidgetClass))
	{
		ShowedTutorialView = Widget;
		Widget->AddToViewport(100);

		OnOpenedWindow.Broadcast();
		OnOpenedWindowDynamic.Broadcast();
	}
}

void UMetaGameSubsystem::CloseTutorialWidget()
{
	if (ShowedTutorialView != nullptr)
	{
		ShowedTutorialView->RemoveFromParent();
		ShowedTutorialView = nullptr;

		OnClosedWindow.Broadcast();
		OnClosedWindowDynamic.Broadcast();
	}
}

bool UMetaGameSubsystem::IsFighterLockedForTurn(FName FighterID)
{
	return LockedUnitsForOneTurn.Contains(FighterID);
}

void UMetaGameSubsystem::ShowDialogue(FName DialogueID)
{
	auto DialogueDT = GetDialogueDataTable();
	ensure(DialogueDT);

	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->DialogueWidget.LoadSynchronous();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!WidgetClass || !PC)
		return;


	if (UBaseMetaDialogueWidget* Widget = CreateWidget<UBaseMetaDialogueWidget>(PC, WidgetClass))
	{
		ShowedDialogueView = Widget;
		Widget->Initialize(DialogueDT, DialogueID);
		Widget->AddToViewport(100);

		OnOpenedWindow.Broadcast();
		OnOpenedWindowDynamic.Broadcast();
	}
}

void UMetaGameSubsystem::CloseDialogue()
{
	if (ShowedDialogueView != nullptr)
	{
		ShowedDialogueView->RemoveFromParent();
		ShowedDialogueView = nullptr;

		OnClosedWindow.Broadcast();
		OnClosedWindowDynamic.Broadcast();
	}
}


void UMetaGameSubsystem::IgnoreNotAssignedActivities()
{
	if (IsCurrentTurnHasMission())
	{
		StartMission();
	}
	else
	{
		ResolveActivities();
		PerformNextTurn();
	}
}

void UMetaGameSubsystem::StartMission()
{
	ensure(DataManager);
	FMetaGame_PreparedActionData ActionData;

	for (const auto& Action : ActionsToResolve)
	{
		if (DataManager->IsMissionNodesContainsID(Action.ID))
		{
			ActionData = Action;
			break;
		}
	}

	if (!DataManager->IsMissionNodesContainsID(ActionData.ID)) return;

	const auto MissionNodeData = DataManager->GetMissionNode(ActionData.ID);
	if (MissionNodeData == nullptr) return;

	RequestedNextTurn = true;
	ActionsToResolve.Remove(ActionData);

	SaveMeta();

	if (const auto Data = Cast<UMetaGame_NodeAction_MissionData>(MissionNodeData->NodeActionDataAsset))
	{
		UT01GameInstance* GI = Cast<UT01GameInstance>(GetWorld()->GetGameInstance());
		if (GI == nullptr) return;

		TArray<FString> FightersToSpawn;
		for (auto FighterData : ActionData.FightersData)
		{
			FightersToSpawn.Add(FighterData.ID.ToString());
		}

		GI->SetUnitsToSpawn(FightersToSpawn);
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, Data->Map);
	}
}

void UMetaGameSubsystem::CloseActivityUI()
{
	if (ShowedActivityView == nullptr) return;
	ShowedActivityView->RemoveFromParent();
	ShowedActivityView = nullptr;
	OnClosedWindow.Broadcast();
	OnClosedWindowDynamic.Broadcast();
}

void UMetaGameSubsystem::ShowMissionUI(FName NodeID, bool& Success)
{
	Success = false;

	if (ShowedMissionView != nullptr) return;

	const auto NodeData = GetNodeData(NodeID);
	const auto NodeActionInstance = MetaMapSubsystem->GetNodeActionInstance(NodeID);
	if (NodeActionInstance == nullptr) return;
	if (NodeData.NodeType != EMetaGame_MapNodeType::Activity) return;
	const auto NodeMissionInstance = Cast<UMetaGame_NodeAction_Mission>(NodeActionInstance);
	if (NodeMissionInstance == nullptr) return;

	if (NodeStates[NodeID] == EMetaGame_MapNodeState::Completed) return;

	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	UClass* WidgetClass = MetaGameSettings->MissionViewWidget.LoadSynchronous();
	const UWorld* World = GetWorld();
	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);

	if (!WidgetClass || !PC)
	{
		return;
	}

	UBaseMetaMissionWidget* Widget = CreateWidget<UBaseMetaMissionWidget>(PC, WidgetClass);
	if (!Widget)
	{
		Success = false;
		return;
	}

	const auto NodeToResolveData = ActionsToResolve.FindByPredicate([NodeID](const FMetaGame_PreparedActionData& ResolveData)
		{
			return ResolveData.ID == NodeID;
		}
	);

	auto DataAsset = Cast<UMetaGame_NodeAction_MissionData>(NodeMissionInstance->DataAsset);
	if (DataAsset != nullptr && DataAsset->ForcedFighters.Num() > 0)
	{
		TArray<FMetaGame_FighterData> Fighters = TArray<FMetaGame_FighterData>();
		if (NodeToResolveData != nullptr)
		{
			Fighters = NodeToResolveData->FightersData;
		}

		TArray<FMetaGame_FighterData> ForcedFightersData;
		for (auto FighterID : DataAsset->ForcedFighters)
		{
			auto Found = AllFighters.FindByPredicate([FighterID](const FMetaGame_FighterData& Data)
			{
				return FighterID == Data.ID;
			});
			if (Found)
			{
				ForcedFightersData.Add(*Found);
			}
		}

		Widget->InitializeWithFighters(NodeMissionInstance, Fighters, ForcedFightersData);
	}
	else
	{
		if (NodeToResolveData != nullptr)
		{
			const auto Fighters = NodeToResolveData->FightersData;
			Widget->InitializeWithFighters(NodeMissionInstance, Fighters, TArray<FMetaGame_FighterData>());
		}
		else
		{
			Widget->Initialize(NodeMissionInstance);
		}
	}

	Widget->AddToViewport(100);
	ShowedMissionView = Widget;
	Success = true;

	OnOpenedWindow.Broadcast();
	OnOpenedWindowDynamic.Broadcast();
}

void UMetaGameSubsystem::CloseMissionUI()
{
	if (ShowedMissionView == nullptr) return;
	ShowedMissionView->RemoveFromParent();
	ShowedMissionView = nullptr;
	OnClosedWindow.Broadcast();
	OnClosedWindowDynamic.Broadcast();
}

void UMetaGameSubsystem::CloseLoreUI()
{
	if (ShowedLoreView == nullptr) return;
	ShowedLoreView->RemoveFromParent();
	ShowedLoreView = nullptr;
	OnClosedWindow.Broadcast();
	OnClosedWindowDynamic.Broadcast();
}


void UMetaGameSubsystem::NextTurnClicked()
{
	auto NextTurnStatus = GetNextTurnStatus();

	switch (NextTurnStatus)
	{
	case EMetaGame_NextTurnStatus::NoMoreTurns:
		{
			//Show Warning;
			return;
		}
	case EMetaGame_NextTurnStatus::NotAssignedMission:
		{
			ShowMissionNotAssignedUI();
			return;
		}
	case EMetaGame_NextTurnStatus::NotAssignedActivities:
		{
			const FMetaGame_TurnData CurrentTurnData = GetAllTurns()[CurrentTurnIndex];
			auto RequiredActivities = CurrentTurnData.RequiredNodeIDs;

			bool HasUnpreparedRequiredActions = false;
			for (FName RequiredActivity : RequiredActivities)
			{
				const bool Contains = ActionsToResolve.ContainsByPredicate([RequiredActivity](const FMetaGame_PreparedActionData& ResolveData)
				{
					return ResolveData.ID == RequiredActivity;
				});
				if (!Contains)
				{
					HasUnpreparedRequiredActions = true;
					break;
				}
			}
			ShowActivitiesNotAssignedUI(!HasUnpreparedRequiredActions);
			return;
		}
	case EMetaGame_NextTurnStatus::AllGood:
		{
			if (IsCurrentTurnHasMission())
			{
				StartMission();
			}
			else
			{
				ResolveActivities();
				PerformNextTurn();
			}
			break;
		}
	}
}

void UMetaGameSubsystem::PerformNextTurn()
{
	ensure(DataManager);

	RequestedNextTurn = false;
	LockedUnitsForOneTurn.Empty();
	ActionsToResolve.Empty();

	auto Turns = GetAllTurns();

	if (Turns.Num() - 1 < CurrentTurnIndex) return;
	FMetaGame_TurnData CurrentTurnData = Turns[CurrentTurnIndex];

	FName SquadPosNodeID = CurrentTurnData.SquadPositionNodeID;

	auto SquadNode = DataManager->GetSquadPosition(SquadPosNodeID);
	if (SquadNode == nullptr) return;

	if (Turns.Num() - 1 < CurrentTurnIndex + 1) return;
	FMetaGame_TurnData HeadTurnData = Turns[CurrentTurnIndex + 1];

	FName NextSquadPosID = HeadTurnData.SquadPositionNodeID;

	auto HeadSquadNode = DataManager->GetSquadPosition(NextSquadPosID);
	if (HeadSquadNode == nullptr) return;


	TWeakObjectPtr<UMetaGameSubsystem> WeakThis(this);
	MetaMapSubsystem->OnTransitionComplete.AddLambda([WeakThis]
	{
		WeakThis->UpdateMap();
		WeakThis->CheckTurnLore();
		WeakThis->CheckTurnDialogue();
		WeakThis->CheckTurnTutorial();
		WeakThis->SaveMeta();
	});
	RemoveGottenRewardNotificationFromLastTurn();
	CurrentTurnIndex++;
	OnTurnChanged.Broadcast();
	OnTurnChangedDynamic.Broadcast();
	MetaMapSubsystem->StartTransition(SquadNode, HeadSquadNode->WorldPosition);
}

void UMetaGameSubsystem::NextTurnOnStart()
{
	ResolveActivities();
	PerformNextTurn();
}

EMetaGame_NextTurnStatus UMetaGameSubsystem::GetNextTurnStatus()
{
	ensure(DataManager);

	TArray<FMetaGame_TurnData> Turns = GetAllTurns();
	if (Turns.Num() - 1 < CurrentTurnIndex) return EMetaGame_NextTurnStatus::NoMoreTurns;
	FMetaGame_TurnData CurrentTurnData = Turns[CurrentTurnIndex];

	TArray<FName> MissionIDs = TArray<FName>();
	TArray<FName> ActivityIDs = TArray<FName>();

	for (auto& AvailableNode : CurrentTurnData.AvailableNodeIDs)
	{
		if (DataManager->IsMissionNodesContainsID(AvailableNode)) MissionIDs.AddUnique(AvailableNode);
		if (DataManager->IsActivityNodesContainsID(AvailableNode)) ActivityIDs.AddUnique(AvailableNode);
	}

	bool AllMissionsPrepared = true;
	for (FName MissionID : MissionIDs)
	{
		bool IsMissionPrepared = ActionsToResolve.ContainsByPredicate([MissionID](const FMetaGame_PreparedActionData& Data)
		{
			return Data.ID == MissionID;
		});
		if (!IsMissionPrepared) AllMissionsPrepared = false;
	}

	if (!AllMissionsPrepared) return EMetaGame_NextTurnStatus::NotAssignedMission;

	bool AllActivitiesPrepared = true;
	for (FName ActivityID : ActivityIDs)
	{
		bool IsActivityPrepared = ActionsToResolve.ContainsByPredicate([ActivityID](const FMetaGame_PreparedActionData& Data)
		{
			return Data.ID == ActivityID;
		});
		if (!IsActivityPrepared) AllActivitiesPrepared = false;
	}

	if (!AllActivitiesPrepared) return EMetaGame_NextTurnStatus::NotAssignedActivities;

	return EMetaGame_NextTurnStatus::AllGood;
}

bool UMetaGameSubsystem::IsCurrentTurnHasMission()
{
	ensure(DataManager);
	TArray<FMetaGame_TurnData> Turns = GetAllTurns();
	FMetaGame_TurnData CurrentTurnData = Turns[CurrentTurnIndex];

	for (const auto& AvailableNode : CurrentTurnData.AvailableNodeIDs)
	{
		if (DataManager->IsMissionNodesContainsID(AvailableNode)) return true;
	}
	return false;
}

int UMetaGameSubsystem::GetCurrentTurnIndex()
{
	return CurrentTurnIndex;
}

bool UMetaGameSubsystem::IsCurrentTurnSquadPositionNode(FName ID)
{
	return GetAllTurns()[CurrentTurnIndex].SquadPositionNodeID == ID;
}

void UMetaGameSubsystem::OnNodeClicked(FName ID)
{
	//TODO: Lock map nodes if already have active node
	ActivateNode(ID);
}

void UMetaGameSubsystem::ForceCompleteNode(FName ID)
{
	NodeStates[ID] = EMetaGame_MapNodeState::Completed;
	UpdateMap();
}


void UMetaGameSubsystem::ActivateNode(FName ID)
{
	if (!MetaMapSubsystem) return;

	auto State = NodeStates.Find(ID);
	if (State == nullptr) return;

	if (NodeStates[ID] != EMetaGame_MapNodeState::Locked)
	{
		const FMetaGame_MapNodeData Data = GetNodeData(ID);

		auto NodeActionInstance = MetaMapSubsystem->GetNodeActionInstance(ID);

		if (NodeActionInstance != nullptr)
		{
			NodeActionInstance->ExecuteNodeAction(this);
			//TODO: META GAME: Remove force complete as a thing
			if (Data.NodeType != EMetaGame_MapNodeType::Activity) ForceCompleteNode(ID);
		}
	}
}

void UMetaGameSubsystem::QueueActivity(FName ID, const TArray<FMetaGame_FighterData>& Fighters)
{
	FMetaGame_PreparedActionData PreparedData;
	PreparedData.ID = ID;
	PreparedData.FightersData = Fighters;
	PreparedData.Scheduled = FDateTime::Now();

	bool HasActivityQueued = false;
	int Indx = -1;
	for (int i = 0; i < ActionsToResolve.Num(); i++)
	{
		if (ActionsToResolve[i].ID.IsEqual(ID))
		{
			HasActivityQueued = true;
			Indx = i;
		}
	}

	const FMetaGame_MapNodeData Data = GetNodeData(ID);
	auto NodeActionInstance = MetaMapSubsystem->GetNodeActionInstance(ID);
	if (NodeActionInstance == nullptr)
	{
		return;
	}
	if (Data.NodeType != EMetaGame_MapNodeType::Activity)
	{
		return;
	}
	auto ActivityInstance = Cast<UMetaGame_NodeAction_Activity>(NodeActionInstance);
	if (ActivityInstance != nullptr)
	{
		bool ResolveSuccess = false;
		TMap<FName, int> Rewards;
		bool FullResolve;
		ActivityInstance->ResolveActivity(Fighters, ResolveSuccess, FullResolve, Rewards);

		if (!ResolveSuccess)
		{
			ShowActivityCantBeResolved();
			return;
		}
	}

	if (HasActivityQueued)
	{
		if (PreparedData.FightersData.Num() <= 0)
		{
			ActionsToResolve.RemoveAt(Indx);
		}
		else
		{
			ActionsToResolve[Indx] = PreparedData;
		}
	}
	else
	{
		if (PreparedData.FightersData.Num() > 0)
		{
			ActionsToResolve.Add(PreparedData);
		}
	}


	ActionsToResolve.Sort([](const FMetaGame_PreparedActionData& A, const FMetaGame_PreparedActionData& B)
	{
		return A.Scheduled >= B.Scheduled;
	});

	OnPendingActivitiesUpdatedDynamic.Broadcast();
	OnPendingActivitiesUpdated.Broadcast();

	if (NodeStates.Contains(ID))
	{
		NodeStates[ID] = EMetaGame_MapNodeState::Pending;
	}
	else
	{
		NodeStates.Add(ID, EMetaGame_MapNodeState::Pending);
	}

	UpdateMap();
	CloseActivityUI();
	CloseMissionUI();
}

void UMetaGameSubsystem::RemoveFromPendingActivity(FName ID)
{
	int FoundIdx = -1;
	for (int i = 0; i < ActionsToResolve.Num(); i++)
	{
		if (ActionsToResolve[i].ID.IsEqual(ID)) FoundIdx = i;
	}
	if (FoundIdx >= 0)
	{
		ActionsToResolve.RemoveAt(FoundIdx);
		OnPendingActivitiesUpdatedDynamic.Broadcast();
		OnPendingActivitiesUpdated.Broadcast();
	}
}

void UMetaGameSubsystem::ResolveActivities()
{
	if (!MetaMapSubsystem) return;

	TArray<FMetaGame_PreparedActionData> ResolvedActivities;
	ResolvedActivities.Empty();

	if (ActionsToResolve.Num() == 0) return;

	for (int i = 0; i < ActionsToResolve.Num(); ++i)
	{
		const FMetaGame_MapNodeData Data = GetNodeData(ActionsToResolve[i].ID);
		auto NodeActionInstance = MetaMapSubsystem->GetNodeActionInstance(ActionsToResolve[i].ID);
		if (NodeActionInstance == nullptr)
		{
			return;
		}
		if (Data.NodeType != EMetaGame_MapNodeType::Activity)
		{
			return;
		}
		auto ActivityInstance = Cast<UMetaGame_NodeAction_Activity>(NodeActionInstance);
		if (ActivityInstance != nullptr)
		{
			bool ResolveSuccess = false;
			bool FullResolve = false;
			TMap<FName, int> Rewards;
			ActivityInstance->ResolveActivity(ActionsToResolve[i].FightersData, ResolveSuccess, FullResolve, Rewards);

			if (ResolveSuccess)
			{
				auto ActivityData = Cast<UMetaGame_NodeAction_ActivityData>(ActivityInstance->DataAsset);
				FText CompletionText = FText::FromString("");
				if (ActivityData != nullptr)
				{
					CompletionText = FullResolve ? ActivityData->FullCompletionText : ActivityData->PartialCompletionText;
				}
				FMetaGame_RewardNotificationData GottenRewardData;
				GottenRewardData.GUID = FGuid::NewGuid();
				GottenRewardData.ActivityID = Data.ID;
				GottenRewardData.GetOnTurnNum = CurrentTurnIndex;
				GottenRewardData.NotificationGetTime = FDateTime::Now();
				GottenRewardData.Rewards = Rewards;
				GottenRewardData.CompletionText = CompletionText;
				GottenRewards.Add(GottenRewardData);

				GottenRewards.Sort([](const FMetaGame_RewardNotificationData& Data1, const FMetaGame_RewardNotificationData& Data2)
				{
					return Data1.NotificationGetTime < Data2.NotificationGetTime;
				});

				for (auto Reward : Rewards)
				{
					auto RewardData = GetRewardData(Reward.Key);
					if (RewardData.Type == EMetaGame_RewardResourceType::Resource)
					{
						if (Resources.Contains(Reward.Key))
						{
							Resources[Reward.Key] += Reward.Value;
						}
						else
						{
							Resources.Add(Reward.Key, Reward.Value);
						}
					}
					else
					{
						for (int j = 0; j < Reward.Value; ++j)
						{
							ULootItemInstance* Item;
							StorageInventory->InventoryComponent->TryAddItemWithAutoPlace(RewardData.ItemId, RewardData.ItemCount, EInventoryContainerType::Ground, EContainerItemUpdateReason::AutoPlaced, true, Item);
						}
					}
				}

				OnGottenRewardsUpdated.Broadcast();
				OnGottenRewardsUpdatedDynamic.Broadcast();

				ResolvedActivities.Add(ActionsToResolve[i]);
				NodeStates[ActionsToResolve[i].ID] = EMetaGame_MapNodeState::Completed;
			}
		}
	}

	for (auto Resolved : ResolvedActivities)
	{
		ActionsToResolve.Remove(Resolved);
	}

	OnPendingActivitiesUpdatedDynamic.Broadcast();
	OnPendingActivitiesUpdated.Broadcast();

	UpdateMap();
	CloseActivityUI();
	SaveMeta();
}

void UMetaGameSubsystem::OnLoreScenarioClick(FName LoreID, FMetaGame_LoreScenarioData Data)
{
	if (LoreID.IsNone()) return;

	CompletedLoreIDs.Add(LoreID);

	switch (Data.ActionType)
	{
	case EMetaGame_LoreScenarioType::TakeResources:
		{
			if (!Data.ResourceID.IsNone() && Data.ResourceCount > 0)
			{
				if (Resources.Contains(Data.ResourceID) && Resources[Data.ResourceID] >= Data.ResourceCount)
				{
					Resources[Data.ResourceID] -= Data.ResourceCount;
				}
				else
				{
					return;
				}
			}
			break;
		}
	case EMetaGame_LoreScenarioType::LockUnit:
		{
			if (Data.UnitsID.Num() > 0)
			{
				for (auto UnitID : Data.UnitsID)
				{
					if (!UnitID.IsNone())
					{
						LockedUnitsForOneTurn.Add(UnitID);
					}
				}
			}
			break;
		}
	default: break;
	}

	CloseLoreUI();
	SaveMeta();
}


FMetaGame_MapNodeData UMetaGameSubsystem::GetNodeData(FName ID) const
{
	if (DataManager != nullptr)
	{
		if (const auto ActivityNodeData = DataManager->GetActivityNode(ID); ActivityNodeData != nullptr)
		{
			return *ActivityNodeData;
		}

		if (const auto MissionNodeData = DataManager->GetMissionNode(ID); MissionNodeData != nullptr)
		{
			return *MissionNodeData;
		}

		if (const auto LoreNodeData = DataManager->GetLoreNode(ID); LoreNodeData != nullptr)
		{
			return *LoreNodeData;
		}
	}

	return FMetaGame_MapNodeData();
}

TArray<FMetaGame_RewardData> UMetaGameSubsystem::GetAllRewardsData()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto RewardsDT = MetaGameSettings->RewardsDataTable.LoadSynchronous();

	if (RewardsDT == nullptr) return TArray<FMetaGame_RewardData>();

	TArray<FMetaGame_RewardData*> RewardPtrs;
	RewardsDT->GetAllRows<FMetaGame_RewardData>(TEXT("UMetaGameSubsystem::GetRewardData"), RewardPtrs);
	TArray<FMetaGame_RewardData> Rewards;
	for (const auto RewardPtr : RewardPtrs)
	{
		if (RewardPtr != nullptr) Rewards.Add(*RewardPtr);
	}

	return Rewards;
}

FMetaGame_RewardData UMetaGameSubsystem::GetRewardData(FName ID)
{
	auto AllRewards = GetAllRewardsData();
	const auto FoundReward = AllRewards.FindByPredicate([ID](const FMetaGame_RewardData& RewardData)
	{
		return RewardData.ID == ID;
	});

	return FoundReward == nullptr
		       ? FMetaGame_RewardData()
		       : *FoundReward;
}

TArray<FMetaGame_FighterData> UMetaGameSubsystem::GetAllFightersData()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto FightersDT = MetaGameSettings->FightersDataTable.LoadSynchronous();

	if (FightersDT == nullptr) return TArray<FMetaGame_FighterData>();

	TArray<FMetaGame_FighterData*> FighterPtrs;
	FightersDT->GetAllRows<FMetaGame_FighterData>(TEXT("UMetaGameSubsystem::GetAllFightersData"), FighterPtrs);
	TArray<FMetaGame_FighterData> Fighters;
	for (const auto FighterPtr : FighterPtrs)
	{
		if (FighterPtr != nullptr) Fighters.Add(*FighterPtr);
	}

	return Fighters;
}

FMetaGame_FighterData UMetaGameSubsystem::GetFighterData(FName ID)
{
	const auto FoundFighter = AllFighters.FindByPredicate([ID](const FMetaGame_FighterData& FighterData)
	{
		return FighterData.ID == ID;
	});

	return FoundFighter == nullptr ? FMetaGame_FighterData() : *FoundFighter;
}

TArray<FMetaGame_ThreatData> UMetaGameSubsystem::GetAllThreatsData()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto ThreatsDT = MetaGameSettings->ThreatsDataTable.LoadSynchronous();

	if (ThreatsDT == nullptr) return TArray<FMetaGame_ThreatData>();

	TArray<FMetaGame_ThreatData*> ThreatsPtrs;
	ThreatsDT->GetAllRows<FMetaGame_ThreatData>(TEXT("UMetaGameSubsystem::GetAllThreatsData"), ThreatsPtrs);
	TArray<FMetaGame_ThreatData> Threats;
	for (const auto ThreatPtr : ThreatsPtrs)
	{
		if (ThreatPtr != nullptr) Threats.Add(*ThreatPtr);
	}

	return Threats;
}

FMetaGame_ThreatData UMetaGameSubsystem::GetThreatData(FName ID)
{
	auto Threats = GetAllThreatsData();
	const auto FoundThreat = Threats.FindByPredicate([ID](const FMetaGame_ThreatData& ThreatData)
	{
		return ThreatData.ID == ID;
	});

	return FoundThreat == nullptr ? FMetaGame_ThreatData() : *FoundThreat;
}

TArray<FMetaGame_SkillData> UMetaGameSubsystem::GetAllSkillsData()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto SkillsDT = MetaGameSettings->SkillsDataTable.LoadSynchronous();

	if (SkillsDT == nullptr) return TArray<FMetaGame_SkillData>();

	TArray<FMetaGame_SkillData*> SkillsPtrs;
	SkillsDT->GetAllRows<FMetaGame_SkillData>(TEXT("UMetaGameSubsystem::GetAllThreatsData"), SkillsPtrs);
	TArray<FMetaGame_SkillData> Skills;
	for (const auto SkillPtr : SkillsPtrs)
	{
		if (SkillPtr != nullptr) Skills.Add(*SkillPtr);
	}

	return Skills;
}

FMetaGame_SkillData UMetaGameSubsystem::GetSkillData(FName ID, int32 Level)
{
	auto Skills = GetAllSkillsData();
	const auto FoundSkill = Skills.FindByPredicate([ID, Level](const FMetaGame_SkillData& SkillData)
	{
		return SkillData.ID == ID && SkillData.Level == Level;
	});

	return FoundSkill == nullptr ? FMetaGame_SkillData() : *FoundSkill;
}

TArray<FMetaGame_LoreData> UMetaGameSubsystem::GetAllLoreData()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto LoreDT = MetaGameSettings->LoreDataTable.LoadSynchronous();

	TArray<FMetaGame_LoreData*> LoreDataPtrs;
	TArray<FMetaGame_LoreData> LoreDatas;
	LoreDT->GetAllRows<FMetaGame_LoreData>(TEXT("UMetaGameSubsystem::GetAllLoreData"), LoreDataPtrs);
	for (const auto LoreDataPtr : LoreDataPtrs)
	{
		if (LoreDataPtr != nullptr) LoreDatas.Add(*LoreDataPtr);
	}
	return LoreDatas;
}

FMetaGame_LoreData UMetaGameSubsystem::GetLoreData(FName ID)
{
	auto LoreDatas = GetAllLoreData();
	const auto FoundLore = LoreDatas.FindByPredicate([ID](const FMetaGame_LoreData& LoreData)
	{
		return LoreData.ID == ID;
	});

	return FoundLore == nullptr ? FMetaGame_LoreData() : *FoundLore;
}

TArray<FMetaGame_TutorialData> UMetaGameSubsystem::GetAllTutorialData()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto TutorialDT = MetaGameSettings->TutorialsDataTable.LoadSynchronous();

	TArray<FMetaGame_TutorialData*> TutorialDataPtrs;
	TArray<FMetaGame_TutorialData> TutorialDatas;
	TutorialDT->GetAllRows<FMetaGame_TutorialData>(TEXT("UMetaGameSubsystem::GetAllTutorialData"), TutorialDataPtrs);
	for (const auto TutorialDataPtr : TutorialDataPtrs)
	{
		if (TutorialDataPtr != nullptr) TutorialDatas.Add(*TutorialDataPtr);
	}
	return TutorialDatas;
}

FMetaGame_TutorialData UMetaGameSubsystem::GetTutorialData(FName ID)
{
	auto TutorialDatas = GetAllTutorialData();
	const auto FoundTutorial = TutorialDatas.FindByPredicate([ID](const FMetaGame_TutorialData& TutorialData)
	{
		return TutorialData.ID == ID;
	});

	return FoundTutorial == nullptr ? FMetaGame_TutorialData() : *FoundTutorial;
}

UDataTable* UMetaGameSubsystem::GetDialogueDataTable()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	return MetaGameSettings->DialoguesDataTable.LoadSynchronous();
}


FString UMetaGameSubsystem::GetResourcesString()
{
	FString ResourcesString = FString();
	ResourcesString.Append("| ");

	for (auto Resource : Resources)
	{
		ResourcesString.Append(Resource.Key.ToString());
		ResourcesString.Append(": ");
		ResourcesString.Append(FString::FromInt(Resource.Value));
		ResourcesString.Append(" | ");
	}

	return ResourcesString;
}

FText UMetaGameSubsystem::GetTurnDisplayName()
{
	auto Turns = GetAllTurns();
	auto CurrentTurnData = Turns[CurrentTurnIndex];
	return CurrentTurnData.TurnDisplayName;
}

TMap<FName, int> UMetaGameSubsystem::GetResources()
{
	return Resources;
}


TArray<FMetaGame_TurnData> UMetaGameSubsystem::GetAllTurns() const
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	auto TurnsDT = MetaGameSettings->TurnsDataTable.LoadSynchronous();

	if (TurnsDT == nullptr) return TArray<FMetaGame_TurnData>();

	TArray<FMetaGame_TurnData*> TurnPtrs;
	TurnsDT->GetAllRows<FMetaGame_TurnData>(TEXT("UMetaGameSubsystem::SetupMap"), TurnPtrs);
	TArray<FMetaGame_TurnData> Turns;
	for (const auto TurnPtr : TurnPtrs)
	{
		if (TurnPtr != nullptr) Turns.Add(*TurnPtr);
	}

	return Turns;
}
