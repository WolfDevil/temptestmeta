#pragma once
#include "CommonUserWidget.h"
#include "BaseMetaDialogueWidget.generated.h"

UCLASS(BlueprintType)
class UBaseMetaDialogueWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Initialize(UDataTable* DialoguesDT, FName DialogueID);
};
