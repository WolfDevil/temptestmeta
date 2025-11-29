#pragma once
#include "MetaGame_LoreScenarioData.h"
#include "MetaGame_LoreData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_LoreData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText OnImageText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMetaGame_LoreScenarioData> Actions;

	
};
