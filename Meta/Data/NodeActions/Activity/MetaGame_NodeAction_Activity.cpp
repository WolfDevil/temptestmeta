// Fill out your copyright notice in the Description page of Project Settings.


#include "MetaGame_NodeAction_Activity.h"

#include "MetaGame_NodeAction_ActivityData.h"
#include "T01/Core/Settings/Meta/MetaGameSettings.h"
#include "T01/Core/Subsystem/Meta/MetaGameSubsystem.h"


void UMetaGame_NodeAction_Activity::ExecuteNodeAction(UObject* Context)
{
	//Show UI
	UMetaGameSubsystem* MetaGame = Cast<UMetaGameSubsystem>(Context);
	if (!MetaGame) return;
	if (!DataAsset) return;

	//TODO: META GAME: Some more conditions, like if activity already started resolving (if we are going to have delayed resolve)

	bool WidgetSpawned = false;
	MetaGame->ShowActivityUI(NodeID, WidgetSpawned);
}

void UMetaGame_NodeAction_Activity::ResolveActivity(const TArray<FMetaGame_FighterData>& Fighters, bool& Success, bool& FullCompletion, TMap<FName, int>& Rewards)
{
	if (!DataAsset)
	{
		return;
	}

	Rewards.Empty();

	UMetaGame_NodeAction_ActivityData* Data = Cast<UMetaGame_NodeAction_ActivityData>(DataAsset);
	if (!Data)
	{
		return;
	}

	auto ThreatsData = GetThreatsData();
	bool bAllThreatsPassed = true;
	TArray<FName> PassedThreats;
	PassedThreats.Empty();

	TArray<FName> RequiredThreats;
	RequiredThreats.Empty();
	for (auto Thr : Data->Threats)
	{
		if (Thr.Value)
		{
			RequiredThreats.AddUnique(Thr.Key);
		}
	}

	//Check all threats
	for (const auto& Threat : Data->Threats)
	{
		const auto ThreatData = ThreatsData.FindByPredicate([Threat](const FMetaGame_ThreatData& ThreatData)
		{
			return ThreatData.ID == Threat.Key;
		});

		if (ThreatData)
		{
			if (IsFightersHasResolvingSkill(ThreatData->RequiredSkillID, ThreatData->RequiredSkillLevel, Fighters))
			{
				PassedThreats.Add(Threat.Key);
			}
			else
			{
				bAllThreatsPassed = false;
			}
		}
		else
		{
			bAllThreatsPassed = false;
		}
	}

	//Add reward for threat stages
	for (auto RewardStage : Data->Rewards)
	{
		if (RewardStage.ClosedThreatsAmount <= PassedThreats.Num())
		{
			if (Rewards.Contains(RewardStage.RewardID))
			{
				Rewards[RewardStage.RewardID] += RewardStage.RewardAmount;
			}
			else
			{
				Rewards.Add(RewardStage.RewardID, RewardStage.RewardAmount);
			}
		}
	}

	//Add bonus reward if all threats passed
	if (bAllThreatsPassed)
	{
		if (Data->BonusReward.Type == EMetaGame_BonusRewardType::AllThreatsClosed)
		{
			if (Rewards.Contains(Data->BonusReward.RewardID))
			{
				Rewards[Data->BonusReward.RewardID] += Data->BonusReward.RewardAmount;
			}
			else
			{
				Rewards.Add(Data->BonusReward.RewardID, Data->BonusReward.RewardAmount);
			}
		}

		Success = true;
		FullCompletion = true;
	}
	//or check if we passed all required
	else
	{
		bool AllRequiredPassed = true;
		for (FName& RequiredThreat : RequiredThreats)
		{
			if (!PassedThreats.Contains(RequiredThreat))
			{
				AllRequiredPassed = false;
			}
		}
		Success = AllRequiredPassed;
		FullCompletion = false;
	}
}

void UMetaGame_NodeAction_Activity::PrepareUIRewardsData(
	const TArray<FMetaGame_FighterData>& Fighters,
	TArray<FMetaGame_ActivityRewardStageData>& ThreatRewards,
	FMetaGame_ActivityRewardBonusData& BonusReward,
	int& ClosedThreatsCount,
	bool& BonusConditionMet,
	bool& IsRequiredThreatsPassed)
{
	if (!DataAsset) return;

	UMetaGame_NodeAction_ActivityData* Data = Cast<UMetaGame_NodeAction_ActivityData>(DataAsset);
	if (!Data) return;

	auto ThreatsData = GetThreatsData();

	TArray<FName> PassedThreats;
	PassedThreats.Empty();

	TArray<FName> RequiredThreats;
	RequiredThreats.Empty();
	for (auto Thr : Data->Threats)
	{
		if (Thr.Value)
		{
			RequiredThreats.AddUnique(Thr.Key);
		}
	}

	bool bAllThreatsPassed = true;
	for (const auto& Threat : Data->Threats)
	{
		const auto ThreatData = ThreatsData.FindByPredicate([Threat](const FMetaGame_ThreatData& ThreatData)
		{
			return ThreatData.ID == Threat.Key;
		});

		if (ThreatData)
		{
			if (IsFightersHasResolvingSkill(ThreatData->RequiredSkillID, ThreatData->RequiredSkillLevel, Fighters))
			{
				PassedThreats.AddUnique(Threat.Key);
			}
			else
			{
				bAllThreatsPassed = false;
			}
		}
		else
		{
			bAllThreatsPassed = false;
		}
	}

	ThreatRewards.Empty();
	ThreatRewards = Data->Rewards;
	BonusReward = Data->BonusReward;
	ClosedThreatsCount = PassedThreats.Num();

	bool AllRequiredPassed = true;
	for (FName& RequiredThreat : RequiredThreats)
	{
		if (!PassedThreats.Contains(RequiredThreat))
		{
			AllRequiredPassed = false;
		}
	}

	IsRequiredThreatsPassed = AllRequiredPassed;
	BonusConditionMet = Data->BonusReward.Type == EMetaGame_BonusRewardType::AllThreatsClosed && bAllThreatsPassed;
}

void UMetaGame_NodeAction_Activity::PrepareUIThreatsData(const TArray<FMetaGame_FighterData>& Fighters, TMap<FName, bool>& Threats, TArray<FName>& ClosedThreats)
{
	if (!DataAsset) return;

	UMetaGame_NodeAction_ActivityData* Data = Cast<UMetaGame_NodeAction_ActivityData>(DataAsset);
	if (!Data) return;

	auto ThreatsData = GetThreatsData();
	ClosedThreats.Empty();

	for (const auto& Threat : Data->Threats)
	{
		const auto ThreatData = ThreatsData.FindByPredicate([Threat](const FMetaGame_ThreatData& D)
		{
			return D.ID == Threat.Key;
		});

		if (ThreatData)
		{
			if (IsFightersHasResolvingSkill(ThreatData->RequiredSkillID, ThreatData->RequiredSkillLevel, Fighters))
			{
				ClosedThreats.Add(Threat.Key);
			}
		}
	}

	Threats.Empty();
	Threats = Data->Threats;
}

bool UMetaGame_NodeAction_Activity::IsFightersHasResolvingSkill(FName RequiredSkill, int32 RequiredLevel, const TArray<FMetaGame_FighterData>& Fighters) const
{
	bool bSuccess = false;
	for (auto Fighter : Fighters)
	{
		if (Fighter.Skills.Contains(RequiredSkill))
		{
			if (const auto LVL = Fighter.Skills.Find(RequiredSkill); LVL != nullptr)
			{
				if (*LVL >= RequiredLevel)
				{
					bSuccess = true;
					return true;
				}
			}
		}
	}
	return bSuccess;
}

TArray<FMetaGame_ThreatData> UMetaGame_NodeAction_Activity::GetThreatsData()
{
	const UMetaGameSettings* MetaGameSettings = GetDefault<UMetaGameSettings>();
	const auto ThreatsDT = MetaGameSettings->ThreatsDataTable.LoadSynchronous();

	if (ThreatsDT == nullptr) return TArray<FMetaGame_ThreatData>();
	TArray<FMetaGame_ThreatData*> ThreatRows;
	ThreatsDT->GetAllRows<FMetaGame_ThreatData>(TEXT("::GetThreatsData"), ThreatRows);
	TArray<FMetaGame_ThreatData> Threats;
	for (const auto ThreatRow : ThreatRows)
	{
		if (ThreatRow != nullptr) Threats.Add(*ThreatRow);
	}
	return Threats;
}
