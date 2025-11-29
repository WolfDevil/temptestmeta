#pragma once
#include "EMetaGame_BonusRewardType.h"
#include "MetaGame_ActivityRewardBonusData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_ActivityRewardBonusData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMetaGame_BonusRewardType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int RewardAmount;
};
