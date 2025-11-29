#pragma once
#include "MetaGame_SkillData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_SkillData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool ShowInUI = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;
};
