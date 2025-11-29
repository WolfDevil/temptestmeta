#pragma once
#include "MetaGame_ActivityRewardStageData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_ActivityRewardStageData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ClosedThreatsAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RewardAmount;
};
