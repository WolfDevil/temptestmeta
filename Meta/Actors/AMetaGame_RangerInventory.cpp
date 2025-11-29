#include "AMetaGame_RangerInventory.h"

void AMetaGame_RangerInventory::BeginPlay()
{
	Super::BeginPlay();
	InventoryComponent->Initialize();
	InventoryComponent->ManualLoad(UnitConfigName);
}

AMetaGame_RangerInventory::AMetaGame_RangerInventory()
{
	InventoryComponent = CreateDefaultSubobject<URangerInventoryComponent>("InventoryComponent");
}

void AMetaGame_RangerInventory::Initialize(FString InUnitConfigName)
{
	UnitConfigName = InUnitConfigName;
}
