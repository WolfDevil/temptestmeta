// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_Base.h"


#include "MetaGame_NodeAction_Activity.generated.h"

struct FMetaGame_ActivityRewardBonusData;
struct FMetaGame_ActivityRewardStageData;
struct FMetaGame_FighterData;
/**
 * 
 */
UCLASS(Blueprintable)
class T01_API UMetaGame_NodeAction_Activity : public UMetaGame_NodeAction_Base
{
	GENERATED_BODY()

public:
	virtual void ExecuteNodeAction(UObject* Context) override;

	UFUNCTION(BlueprintCallable)
	virtual void ResolveActivity(const TArray<FMetaGame_FighterData>& Fighters, bool& Success, bool& FullCompletion, TMap<FName, int>& Rewards);

	UFUNCTION(BlueprintCallable)
	virtual void PrepareUIRewardsData(
		const TArray<FMetaGame_FighterData>& Fighters,

		TArray<FMetaGame_ActivityRewardStageData>& ThreatRewards,
		FMetaGame_ActivityRewardBonusData& BonusReward,
		int& ClosedThreatsCount,
		bool& BonusConditionMet,
		bool& IsRequiredThreatsPassed
	);

	UFUNCTION(BlueprintCallable)
	virtual void PrepareUIThreatsData(const TArray<FMetaGame_FighterData>& Fighters, TMap<FName, bool>& Threats, TArray<FName>& ClosedThreats);

	UFUNCTION(BlueprintCallable)
	bool IsFightersHasResolvingSkill(FName RequiredSkill, int32 RequiredLevel, const TArray<FMetaGame_FighterData>& Fighters) const;
};
