#pragma once
#include "MetaGame_RewardData.h"
#include "MetaGame_RewardNotificationData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_RewardNotificationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGuid GUID;

	UPROPERTY(BlueprintReadOnly)
	FName ActivityID;

	UPROPERTY(BlueprintReadOnly)
	FText CompletionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int GetOnTurnNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime NotificationGetTime;

	bool operator==(const FMetaGame_RewardNotificationData& Other) const
	{
		return GUID == Other.GUID;
	}
};
