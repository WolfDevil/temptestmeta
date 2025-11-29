#pragma once
#include "NodeActions/Lore/MetaGame_NodeAction_LoreData.h"
#include "MetaGame_TurnData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_TurnData : public FTableRowBase
{
	GENERATED_BODY();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TurnDisplayName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SquadPositionNodeID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TutorialID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DialogueID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> AvailableNodeIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> RequiredNodeIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName StartLoreID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> AvailableFighterIDs;
};
