#pragma once
#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "T01/Core/Inventory/Components/RangerInventoryComponent.h"
#include "BaseMetaInventoryWidget.generated.h"

UCLASS(BlueprintType)
class UBaseMetaInventoryWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent)
	void InitializeWindow(FName FighterID, URangerInventoryComponent* RangerInventoryComponent, UBaseInventoryComponent* StorageInventoryComponent);
	
};
