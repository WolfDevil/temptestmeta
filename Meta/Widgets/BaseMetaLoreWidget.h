#pragma once
#include "CommonUserWidget.h"
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Lore/MetaGame_LoreData.h"
#include "BaseMetaLoreWidget.generated.h"

UCLASS(BlueprintType)
class UBaseMetaLoreWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void InitializeWidget(FMetaGame_LoreData InData);
};
