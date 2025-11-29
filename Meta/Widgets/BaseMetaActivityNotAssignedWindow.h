#pragma once
#include "CommonUserWidget.h"
#include "BaseMetaActivityNotAssignedWindow.generated.h"

UCLASS(BlueprintType)
class UBaseMetaActivityNotAssignedWindow : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Setup(bool bCanIgnore);
};
