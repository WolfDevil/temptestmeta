// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T01/Core/Inventory/Components/BaseInventoryComponent.h"
#include "AMetaGame_StorageInventory.generated.h"

UCLASS()
class T01_API AMetaGame_StorageInventory : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMetaGame_StorageInventory();

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	class UBaseInventoryComponent* InventoryComponent;

	UPROPERTY()
	FString ConfigID;
	
	void Initialize(FString InConfigID);

	void Save();

	void Load();

};
