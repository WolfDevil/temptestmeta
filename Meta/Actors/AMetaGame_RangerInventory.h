#pragma once
#include "T01/Core/Inventory/Components/RangerInventoryComponent.h"
#include "AMetaGame_RangerInventory.generated.h"

UCLASS(BlueprintType)
class AMetaGame_RangerInventory : public AActor
{
protected:
	virtual void BeginPlay() override;

private:
	GENERATED_BODY()

public:
	AMetaGame_RangerInventory();

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	class URangerInventoryComponent* InventoryComponent;

	UPROPERTY()
	FString UnitConfigName;

	void Initialize(FString InUnitConfigName);
};
