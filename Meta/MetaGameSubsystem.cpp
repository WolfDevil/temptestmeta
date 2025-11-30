#include "MetaGameSubsystem.h"

#include "MetaMapSubsystem.h"
#include "Data/EMetaGame_NextTurnStatus.h"
#include "Data/MetaGame_PreparedActionData.h"
#include "Data/MetaGame_TurnData.h"
#include "Data/MetaGame_TutorialData.h"
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
#include "Widgets/BaseMetaActivityWidget.h"
#include "Widgets/BaseMetaLoreWidget.h"
#include "Widgets/BaseMetaMissionWidget.h"


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
		if (DataManager)
		{
			for (const auto& Pair : DataManager->GetCachedRewards())
			{
				const FMetaGame_RewardData* Reward = Pair.Value;
				if (Reward && Reward->Type == EMetaGame_RewardResourceType::Resource)
				{
					Resources.Add(Reward->ID, 200); //TODO: Move "200" to meta settings.
				}
			}
		}
	}

	auto RangerInventoryClass = Settings->RangerInventoryHolderClass.Get();
	RangerInventories.Empty();
	if (DataManager)
	{
		auto AllFighters = DataManager->GetCachedFighters();
		for (auto Fighter : AllFighters)
		{
			FVector Location = FVector::ZeroVector;
			FRotator Rotation(0.0f, 0.0f, 0.0f);
			FActorSpawnParameters SpawnInfo;
			auto Actor = GetWorld()->SpawnActor<AMetaGame_RangerInventory>(RangerInventoryClass, Location, Rotation, SpawnInfo);
			Actor->Initialize(Fighter.Value->ID.ToString());
			RangerInventories.Add(Fighter.Value->ID, Actor);
		}
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
	TArray<FMetaGame_FighterData> Fighters = GetTurnFighters();
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
	if (!ensure(DataManager)) return TArray<FMetaGame_FighterData>();

	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);
	const auto FightersIDs = CurrentTurnData->AvailableFighterIDs;

	TArray<FMetaGame_FighterData> Fighters;
	for (const auto FighterID : FightersIDs)
	{
		const auto FighterData = DataManager->GetFighterData(FighterID);
		if (FighterData) Fighters.Add(*FighterData);
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
	if (!ensure(DataManager)) return;
	if (MetaMapSubsystem == nullptr) return;

	if (DataManager->GetTurnsCount() - 1 < CurrentTurnIndex) return;
	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);

	TArray<const FMetaGame_MapNodeData*> MapNodes;
	const FName SquadPosNodeID = CurrentTurnData->SquadPositionNodeID;


	const FMetaGame_MapNodeData* SquadNode = DataManager->GetSquadPosition(SquadPosNodeID);
	if (SquadNode == nullptr) return;

	MapNodes.Add(SquadNode);


	if (SquadNode->bShowHead)
	{
		if (DataManager->GetTurnsCount() - 1 >= CurrentTurnIndex + 1)
		{
			const auto HeadTurnData = DataManager->GetTurnData(CurrentTurnIndex + 1);
			const FName SquadPosID = HeadTurnData->SquadPositionNodeID;
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
			if (DataManager->GetTurnsCount() - 1 >= TurnIndex)
			{
				const auto TailTurnData = DataManager->GetTurnData(TurnIndex);
				const FName SquadPosID = TailTurnData->SquadPositionNodeID;
				const FMetaGame_MapNodeData* TailHeadSquadNode = DataManager->GetSquadPosition(SquadPosID);
				if (TailHeadSquadNode == nullptr) return;
				MapNodes.Add(TailHeadSquadNode);
			}

			TurnIndex--;
		}
	}

	for (auto NodeID : CurrentTurnData->AvailableNodeIDs)
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

	MetaMapSubsystem->SetupMap(MapNodes, NodeStates, CurrentTurnData->RequiredNodeIDs);
}

void UMetaGameSubsystem::CheckTurnLore()
{
	if (!ensure(DataManager)) return;


	if (DataManager->GetTurnsCount() - 1 < CurrentTurnIndex) return;
	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);
	const FName LoreID = CurrentTurnData->StartLoreID;
	if (LoreID.IsNone()) return;
	if (CompletedLoreIDs.Contains(LoreID)) return;

	bool Success;
	ShowLoreUI(LoreID, Success);
}

void UMetaGameSubsystem::CheckTurnTutorial()
{
	if (!ensure(DataManager)) return;

	if (DataManager->GetTurnsCount() - 1 < CurrentTurnIndex) return;
	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);
	const FName TutorialID = CurrentTurnData->TutorialID;
	if (TutorialID.IsNone()) return;

	ShowTutorialWidget(TutorialID);
}

void UMetaGameSubsystem::CheckTurnDialogue()
{
	if (!ensure(DataManager)) return;

	if (DataManager->GetTurnsCount() - 1 < CurrentTurnIndex) return;
	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);
	const FName DialogueID = CurrentTurnData->DialogueID;
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
	if (!ensure(DataManager)) return;
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

	const auto LoreData = DataManager->GetLoreData(LoreID);
	if (!LoreData)
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

	Widget->InitializeWidget(*LoreData);
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
	if (!ensure(DataManager)) return;

	auto TutorialData = DataManager->GetTutorialData(TutorialID);
	if (!TutorialData) return;
	if (TutorialData->Widget == nullptr) return;
	UClass* WidgetClass = TutorialData->Widget.LoadSynchronous();

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
	if (!ensure(DataManager)) return;
	auto DialogueDT = DataManager->GetDialoguesDataTable();
	if (!ensure(DialogueDT)) return;

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
	if (!ensure(DataManager)) return;
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
	if (!ensure(DataManager)) return;

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
			const auto Fighter = DataManager->GetFighterData(FighterID);
			if (Fighter)
			{
				ForcedFightersData.Add(*Fighter);
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
	if (!ensure(DataManager)) return;

	switch (GetNextTurnStatus())
	{
	case EMetaGame_NextTurnStatus::InternalError:
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
			const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);
			auto RequiredActivities = CurrentTurnData->RequiredNodeIDs;

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
	if (!ensure(DataManager)) return;

	RequestedNextTurn = false;
	LockedUnitsForOneTurn.Empty();
	ActionsToResolve.Empty();

	if (DataManager->GetTurnsCount() - 1 < CurrentTurnIndex) return;
	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);

	const FName SquadPosNodeID = CurrentTurnData->SquadPositionNodeID;

	const auto SquadNode = DataManager->GetSquadPosition(SquadPosNodeID);
	if (SquadNode == nullptr) return;

	if (DataManager->GetTurnsCount() - 1 < CurrentTurnIndex + 1) return;
	const auto HeadTurnData = DataManager->GetTurnData(CurrentTurnIndex + 1);

	const FName NextSquadPosID = HeadTurnData->SquadPositionNodeID;

	const auto HeadSquadNode = DataManager->GetSquadPosition(NextSquadPosID);
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
	if (!ensure(DataManager)) return EMetaGame_NextTurnStatus::InternalError;

	if (DataManager->GetTurnsCount() - 1 < CurrentTurnIndex) return EMetaGame_NextTurnStatus::NoMoreTurns;
	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);

	TArray<FName> MissionIDs = TArray<FName>();
	TArray<FName> ActivityIDs = TArray<FName>();

	for (auto& AvailableNode : CurrentTurnData->AvailableNodeIDs)
	{
		if (DataManager->IsMissionNodesContainsID(AvailableNode)) MissionIDs.AddUnique(AvailableNode);
		if (DataManager->IsActivityNodesContainsID(AvailableNode)) ActivityIDs.AddUnique(AvailableNode);
	}

	bool AllMissionsPrepared = true;
	for (FName MissionID : MissionIDs)
	{
		const bool IsMissionPrepared = ActionsToResolve.ContainsByPredicate([MissionID](const FMetaGame_PreparedActionData& Data)
		{
			return Data.ID == MissionID;
		});
		if (!IsMissionPrepared) AllMissionsPrepared = false;
	}

	if (!AllMissionsPrepared) return EMetaGame_NextTurnStatus::NotAssignedMission;

	bool AllActivitiesPrepared = true;
	for (FName ActivityID : ActivityIDs)
	{
		const bool IsActivityPrepared = ActionsToResolve.ContainsByPredicate([ActivityID](const FMetaGame_PreparedActionData& Data)
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
	if (!ensure(DataManager)) return false;
	const auto CurrentTurnData = DataManager->GetTurnData(CurrentTurnIndex);

	for (const auto& AvailableNode : CurrentTurnData->AvailableNodeIDs)
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
	if (!ensure(DataManager)) return false;
	return DataManager->GetTurnData(CurrentTurnIndex)->SquadPositionNodeID == ID;
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


void UMetaGameSubsystem::ResolveActivities()
{
	if (!MetaMapSubsystem) return;
	if (!ensure(DataManager)) return;

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
					auto RewardData = DataManager->GetRewardData(Reward.Key);
					if (RewardData->Type == EMetaGame_RewardResourceType::Resource)
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
							StorageInventory->InventoryComponent->TryAddItemWithAutoPlace(RewardData->ItemId, RewardData->ItemCount, EInventoryContainerType::Ground, EContainerItemUpdateReason::AutoPlaced, true, Item);
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
	if (!ensure(DataManager)) return FMetaGame_MapNodeData();

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

	return FMetaGame_MapNodeData();
}

FMetaGame_RewardData UMetaGameSubsystem::GetRewardData(FName ID)
{
	if (!DataManager) return FMetaGame_RewardData();
	const FMetaGame_RewardData* Data = DataManager->GetRewardData(ID);
	return Data ? *Data : FMetaGame_RewardData();
}

FMetaGame_FighterData UMetaGameSubsystem::GetFighterData(FName ID)
{
	if (!ensure(DataManager)) return FMetaGame_FighterData();
	const auto Fighter = DataManager->GetFighterData(ID);
	return Fighter == nullptr ? FMetaGame_FighterData() : *Fighter;
}

FMetaGame_ThreatData UMetaGameSubsystem::GetThreatData(FName ID)
{
	if (!ensure(DataManager)) return FMetaGame_ThreatData();
	const auto Threat = DataManager->GetThreatData(ID);
	return Threat == nullptr ? FMetaGame_ThreatData() : *Threat;
}

FMetaGame_SkillData UMetaGameSubsystem::GetSkillData(FName ID, int32 Level)
{
	if (!ensure(DataManager)) return FMetaGame_SkillData();
	const FMetaGame_SkillData* Data = DataManager->GetSkill(ID, Level);
	return Data ? *Data : FMetaGame_SkillData();
}

FText UMetaGameSubsystem::GetTurnDisplayName()
{
	if (!ensure(DataManager)) return FText::GetEmpty();
	return DataManager->GetTurnData(CurrentTurnIndex)->TurnDisplayName;
}

TMap<FName, int> UMetaGameSubsystem::GetResources()
{
	return Resources;
}
