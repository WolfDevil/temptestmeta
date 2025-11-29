// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MetaGame_NodeAction_BaseData.generated.h"


UCLASS(BlueprintType)
class T01_API UMetaGame_NodeAction_BaseData : public UDataAsset
{
	GENERATED_BODY()

public:
	//=== === === ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "General | Detailed View", DisplayName = "Name"))
	FText DetailedView_Name;

	//=== === === ===

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "General | Detailed View", DisplayName = "Image"))
	TSoftObjectPtr<UTexture2D> DetailedView_Image;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "General | Detailed View", DisplayName = "Description"))
	FText DetailedView_Description;

	//=== === === ===
};
