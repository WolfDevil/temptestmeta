#pragma once
#include "MetaGame_FighterData.h"

#include "MetaGame_PreparedActivityData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_PreparedActivityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMetaGame_FighterData> FightersData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDateTime Scheduled;

	bool operator==(const FMetaGame_PreparedActivityData& Other) const
	{
		return ID == Other.ID
			&& FightersData == Other.FightersData
			&& Scheduled == Other.Scheduled;
	}
};
