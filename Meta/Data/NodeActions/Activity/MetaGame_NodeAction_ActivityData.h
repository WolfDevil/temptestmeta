#pragma once
#include "T01/Core/Subsystem/Meta/Data/MetaGame_ActivityRewardBonusData.h"
#include "T01/Core/Subsystem/Meta/Data/MetaGame_ActivityRewardStageData.h"
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_BaseData.h"

#include "MetaGame_NodeAction_ActivityData.generated.h"

/**
 * 
 */
UCLASS()
class T01_API UMetaGame_NodeAction_ActivityData : public UMetaGame_NodeAction_BaseData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Fighters"))
	int MaxFightersCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Rewards"))
	TArray<FMetaGame_ActivityRewardStageData> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Rewards"))
	FMetaGame_ActivityRewardBonusData BonusReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Rewards"))
	FText PartialCompletionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Rewards"))
	FText FullCompletionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Threats"))
	TMap<FName, bool> Threats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Activity | Threats"))
	int MinThreatsRequired = -1;
};
