#pragma once
#include "T01/Core/Subsystem/Meta/Widgets/BaseMetaTutorialWidget.h"
#include "MetaGame_TutorialData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_TutorialData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UBaseMetaTutorialWidget> Widget;

};
