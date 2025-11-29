#pragma once
#include "MetaGame_FighterData.h"

#include "MetaGame_PreparedActionData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_PreparedActionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMetaGame_FighterData> FightersData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FDateTime Scheduled;

	bool operator==(const FMetaGame_PreparedActionData& Other) const
	{
		return ID == Other.ID
			&& FightersData == Other.FightersData
			&& Scheduled == Other.Scheduled;
	}
};
