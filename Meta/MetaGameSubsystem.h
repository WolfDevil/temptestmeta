#pragma once
#include "MetaMapSubsystem.h"
#include "Actors/AMetaGame_RangerInventory.h"
#include "Actors/AMetaGame_StorageInventory.h"
#include "Data/EMetaGame_NextTurnStatus.h"
#include "Data/MetaGame_PreparedActionData.h"
#include "Data/MetaGame_FighterData.h"
#include "Data/MetaGame_TurnData.h"
#include "Data/MetaGame_RewardNotificationData.h"
#include "Data/MetaGame_SkillData.h"
#include "Data/MetaGame_ThreatData.h"
#include "Data/MetaGame_TutorialData.h"
#include "Data/NodeActions/Lore/MetaGame_LoreData.h"
#include "T01/Core/Subsystem/Savegame/ISaveable.h"
#include "T01/Core/Subsystem/Savegame/T01SaveModule_MetaProgression.h"
#include "Widgets/BaseMetaActivityWidget.h"
#include "Widgets/BaseMetaDialogueWidget.h"
#include "Widgets/BaseMetaInventoryWidget.h"
#include "Widgets/BaseMetaLoreWidget.h"
#include "Widgets/BaseMetaMissionWidget.h"
#include "Widgets/BaseMetaTutorialWidget.h"


#include "MetaGameSubsystem.generated.h"

class UMetaGame_DataManager;
DECLARE_MULTICAST_DELEGATE(FMetaGameSubsystemStaticDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMetaGameSubsystemDynamicDelegate);

UCLASS()
class T01_API UMetaGameSubsystem : public UWorldSubsystem, public ISaveable
{
	GENERATED_BODY()
	
public:
	virtual bool CanBeSaved(FString& FailureReason) const override;
	virtual void Save(UT01Save* SaveObject, void* ParentObject) override;
	virtual void Load(UT01Save* SaveObject, void* ParentObject) override;

	void SaveMeta();

	UFUNCTION(BlueprintCallable)
	bool IsInitialized() const;

	UFUNCTION(BlueprintCallable)
	void OnMetaGameModeLoaded();

private:
	UPROPERTY()
	TObjectPtr<UMetaGame_DataManager> DataManager;
	bool bIsInitialized = false;

public:

	UPROPERTY(BlueprintAssignable, DisplayName = "OnPendingActivitiesUpdated")
	FMetaGameSubsystemDynamicDelegate OnPendingActivitiesUpdatedDynamic;
	FMetaGameSubsystemStaticDelegate OnPendingActivitiesUpdated;

	UPROPERTY(BlueprintAssignable, DisplayName = "OnGottenRewardsUpdated")
	FMetaGameSubsystemDynamicDelegate OnGottenRewardsUpdatedDynamic;
	FMetaGameSubsystemStaticDelegate OnGottenRewardsUpdated;

	UPROPERTY(BlueprintAssignable, DisplayName = "OnOpenedWindow")
	FMetaGameSubsystemDynamicDelegate OnOpenedWindowDynamic;
	FMetaGameSubsystemStaticDelegate OnOpenedWindow;

	UPROPERTY(BlueprintAssignable, DisplayName = "OnClosedWindow")
	FMetaGameSubsystemDynamicDelegate OnClosedWindowDynamic;
	FMetaGameSubsystemStaticDelegate OnClosedWindow;

	UPROPERTY(BlueprintAssignable, DisplayName = "OnTurnChanged")
	FMetaGameSubsystemDynamicDelegate OnTurnChangedDynamic;
	FMetaGameSubsystemStaticDelegate OnTurnChanged;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Геттер для менеджера (если другим классам нужен прямой доступ)
	UFUNCTION(BlueprintCallable, Category = "Meta|Data")
	UMetaGame_DataManager* GetDataManager() const { return DataManager; }

	void ApplyMetaProgressionSave(FT01SaveModule_MetaProgression Progression);

	UFUNCTION(BlueprintCallable)
	bool CanControlCamera();

	UFUNCTION(BlueprintCallable)
	bool CanShowHUD();

	UFUNCTION(BlueprintCallable)
	bool CanShowFightersPanel();
	
	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_FighterData> GetAvailableFighters(FName ActivityID);

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_FighterData> GetTurnFighters();

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_FighterData> GetFightersOnActivity(FName ActivityID) const;

	UFUNCTION(BlueprintCallable)
	FMetaGame_MapNodeData GetActivityThatOccupiedFighter(FName FighterID) const;

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_PreparedActionData> GetPendingActivities();

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_RewardNotificationData> GetGottenRewards();

	UFUNCTION(BlueprintCallable)
	bool RemoveGottenRewardNotification(FGuid GUID);

	UFUNCTION(BlueprintCallable)
	void RemoveGottenRewardNotificationFromLastTurn();

	UFUNCTION(BlueprintCallable)
	void UpdateMap();

	UFUNCTION(BlueprintCallable)
	void CheckTurnLore();

	UFUNCTION(BlueprintCallable)
	void CheckTurnTutorial();

	UFUNCTION(BlueprintCallable)
	void CheckTurnDialogue();

	//== UI ======================================================

	//    ActivityUI
	UFUNCTION(BlueprintCallable)
	void ShowActivityUI(FName ID, bool& Success);

	UFUNCTION(BlueprintCallable)
	void CloseActivityUI();

	//    MissionUI
	UFUNCTION(BlueprintCallable)
	void ShowMissionUI(FName NodeID, bool& Success);

	UFUNCTION(BlueprintCallable)
	void CloseMissionUI();

	//    LoreUI
	UFUNCTION(BlueprintCallable)
	void ShowLoreUI(FName LoreID, bool& Success);

	UFUNCTION(BlueprintCallable)
	void ShowLoreUIFromNode(FName NodeID, bool& Success);

	UFUNCTION(BlueprintCallable)
	void CloseLoreUI();

	//    RewardsUI
	UFUNCTION(BlueprintCallable)
	void ShowRewardsWindow(FMetaGame_RewardNotificationData Data);

	// Notifications
	UFUNCTION(BlueprintCallable)
	void ShowMissionNotAssignedUI();

	UFUNCTION(BlueprintCallable)
	void ShowActivitiesNotAssignedUI(bool bCanIgnore);

	UFUNCTION(BlueprintCallable)
	void ShowActivityCantBeResolved();

	//Inventory

	UFUNCTION(BlueprintCallable)
	void OpenFighterInventory(FName FighterID);

	UFUNCTION(BlueprintCallable)
	void OpenFirstFighterInventory();

	UFUNCTION(BlueprintCallable)
	void CloseFighterInventory();

	//Tutorials
	UFUNCTION(BlueprintCallable)
	void ShowTutorialWidget(FName TutorialID);

	UFUNCTION(BlueprintCallable)
	void CloseTutorialWidget();

	// Dialogues UI
	
	UFUNCTION(BlueprintCallable)
	void ShowDialogue(FName DialogueID);

	UFUNCTION(BlueprintCallable)
	void CloseDialogue();
	
	//============================================================


	UFUNCTION(BlueprintCallable)
	bool IsFighterLockedForTurn(FName FighterID);

	
	UFUNCTION(BlueprintCallable)
	void IgnoreNotAssignedActivities();

	UFUNCTION(BlueprintCallable)
	void StartMission();
	
	UFUNCTION(BlueprintCallable)
	void NextTurnClicked();

	UFUNCTION(BlueprintCallable)
	void PerformNextTurn();

	UFUNCTION(BlueprintCallable)
	void NextTurnOnStart();

	UFUNCTION(BlueprintCallable)
	EMetaGame_NextTurnStatus GetNextTurnStatus();

	UFUNCTION(BlueprintCallable)
	bool IsCurrentTurnHasMission();

	UFUNCTION(BlueprintCallable)
	int GetCurrentTurnIndex();

	UFUNCTION(BlueprintCallable)
	bool IsCurrentTurnSquadPositionNode(FName ID);

	UFUNCTION(BlueprintCallable)
	void OnNodeClicked(FName ID);

	UFUNCTION(BlueprintCallable)
	void ForceCompleteNode(FName ID);

	UFUNCTION(BlueprintCallable)
	void ActivateNode(FName ID);

	UFUNCTION(BlueprintCallable)
	void QueueActivity(FName ID, const TArray<FMetaGame_FighterData>& Fighters);

	UFUNCTION(BlueprintCallable)
	void RemoveFromPendingActivity(FName ID);

	UFUNCTION(BlueprintCallable)
	void ResolveActivities();

	UFUNCTION(BlueprintCallable)
	void OnLoreScenarioClick(FName LoreID, FMetaGame_LoreScenarioData Data);

	UFUNCTION(BlueprintCallable)
	FMetaGame_MapNodeData GetNodeData(FName ID) const;

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_RewardData> GetAllRewardsData();
	
	UFUNCTION(BlueprintCallable)
	FMetaGame_RewardData GetRewardData(FName ID);

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_FighterData> GetAllFightersData();
	
	UFUNCTION(BlueprintCallable)
	FMetaGame_FighterData GetFighterData(FName ID);

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_ThreatData> GetAllThreatsData();

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_SkillData> GetAllSkillsData();

	UFUNCTION(BlueprintCallable)
	FMetaGame_ThreatData GetThreatData(FName ID);

	UFUNCTION(BlueprintCallable)
	FMetaGame_SkillData GetSkillData(FName ID, int32 Level);

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_LoreData> GetAllLoreData();
	
	UFUNCTION(BlueprintCallable)
	FMetaGame_LoreData GetLoreData(FName ID);

	UFUNCTION(BlueprintCallable)
	TArray<FMetaGame_TutorialData> GetAllTutorialData();
	
	UFUNCTION(BlueprintCallable)
	FMetaGame_TutorialData GetTutorialData(FName ID);

	UFUNCTION(BlueprintCallable)
	UDataTable* GetDialogueDataTable();

	UFUNCTION(BlueprintCallable)
	FString GetResourcesString();

	UFUNCTION(BlueprintCallable)
	FText GetTurnDisplayName();

	UFUNCTION(BlueprintCallable)
	TMap<FName, int> GetResources();

protected:
	UPROPERTY()
	TObjectPtr<UMetaMapSubsystem> MetaMapSubsystem;
	
	UPROPERTY()
	TArray<FMetaGame_FighterData> AllFighters;

private:
	UPROPERTY()
	TMap<FName, AMetaGame_RangerInventory*> RangerInventories;

	UPROPERTY()
	AMetaGame_StorageInventory* StorageInventory;

	bool RequestedNextTurn;
	
	int CurrentTurnIndex;
	
	TMap<FName, EMetaGame_MapNodeState> NodeStates;
	
	TArray<FMetaGame_PreparedActionData> ActionsToResolve;

	TArray<FMetaGame_RewardNotificationData> GottenRewards;

	TMap<FName, int> Resources;

	TArray<FName> CompletedLoreIDs;

	TArray<FName> LockedUnitsForOneTurn;

	UPROPERTY()
	UBaseMetaActivityWidget* ShowedActivityView;

	UPROPERTY()
	UBaseMetaMissionWidget* ShowedMissionView;

	UPROPERTY()
	UBaseMetaLoreWidget* ShowedLoreView;

	UPROPERTY()
	UBaseMetaInventoryWidget* ShowedInventoryView;

	UPROPERTY()
	UBaseMetaTutorialWidget* ShowedTutorialView;

	UPROPERTY()
	UBaseMetaDialogueWidget* ShowedDialogueView;
	
};
