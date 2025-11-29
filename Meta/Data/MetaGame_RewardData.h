#pragma once
#include "MetaGame_RewardResourceType.h"
#include "MetaGame_RewardData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_RewardData : public FTableRowBase
{
	GENERATED_BODY()

	/** Resources uses this ID as their own  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ID;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMetaGame_RewardResourceType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (GetOptions = "BaseInventoryComponent.GetPossibleItemsStrings", EditCondition = "Type == EMetaGame_RewardResourceType::InventoryItem"))
	FString ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Type == EMetaGame_RewardResourceType::InventoryItem"))
	int ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Name in UI")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Description in UI")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
};
