#pragma once
#include "EMetaGame_LoreScenarioType.h"
#include "MetaGame_LoreScenarioData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_LoreScenarioData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Button Visual"))
	bool ButtonHasIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Button Visual", EditCondition = "ButtonHasIcon", EditConditionHides))
	TSoftObjectPtr<UTexture2D> ButtonIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Button Visual"))
	FText ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Scenario Settings"))
	EMetaGame_LoreScenarioType ActionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Scenario Settings", EditCondition = "ActionType == EMetaGame_LoreScenarioType::LockUnit", EditConditionHides))
	TArray<FName> UnitsID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Scenario Settings", EditCondition = "ActionType == EMetaGame_LoreScenarioType::TakeResources", EditConditionHides))
	FName ResourceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Scenario Settings", EditCondition = "ActionType == EMetaGame_LoreScenarioType::TakeResources", EditConditionHides))
	int ResourceCount;

	
};
