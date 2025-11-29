// Fill out your copyright notice in the Description page of Project Settings.


#include "AMetaGame_StorageInventory.h"


// Sets default values
AMetaGame_StorageInventory::AMetaGame_StorageInventory()
{
	InventoryComponent = CreateDefaultSubobject<UBaseInventoryComponent>("BaseInventoryComponent");
}

void AMetaGame_StorageInventory::Initialize(FString InConfigID)
{
	ConfigID = InConfigID;
	InventoryComponent->Initialize();
}

void AMetaGame_StorageInventory::Save()
{
	InventoryComponent->MetaSave(ConfigID);
}

void AMetaGame_StorageInventory::Load()
{
	InventoryComponent->MetaLoad(ConfigID);
}
