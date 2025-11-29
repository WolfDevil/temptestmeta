#pragma once
#include "MetaGame_FighterData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_FighterData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> FightersPanelIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> MapMiniIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> InventoryPortrait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> Skills;


	int32 GetSkillByThreat(FName Threat) const
	{
		if (const int32* Value = Skills.Find(Threat))
			return *Value;
		return 0;
	}

	bool operator==(const FMetaGame_FighterData& Other) const
	{
		return ID == Other.ID;
	}
};
