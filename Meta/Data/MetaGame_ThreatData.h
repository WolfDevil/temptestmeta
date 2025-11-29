#pragma once
#include "MetaGame_ThreatData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_ThreatData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RequiredSkillID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredSkillLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;
};
