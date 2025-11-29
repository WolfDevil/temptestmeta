#pragma once
#include "MetaGame_MapNode_VisualSettingsData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_MapNode_VisualSettingsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInstance> ShadowMaterial;
};
