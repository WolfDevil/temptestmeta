#pragma once
#include "EMetaGame_MapNodeType.h"
#include "MetaGame_MapNode_VisualSettingsData.h"
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_Base.h"

#include "MetaGame_MapNodeData.generated.h"

USTRUCT(BlueprintType)
struct FMetaGame_MapNodeData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> RequiredNodeIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMetaGame_MapNodeType NodeType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector WorldPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UMetaGame_NodeAction_Base> NodeActionDataClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMetaGame_NodeAction_BaseData* NodeActionDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "NodeType != EMetaGame_MapNodeType::SquadPosition", EditConditionHides))
	FMetaGame_MapNode_VisualSettingsData VisualSettings;

	UPROPERTY(EditAnywhere, meta = (Category = "Squad Position", EditCondition = "NodeType == EMetaGame_MapNodeType::SquadPosition", EditConditionHides))
	bool bShowHead;

	UPROPERTY(EditAnywhere, meta = (Category = "Squad Position", EditCondition = "NodeType == EMetaGame_MapNodeType::SquadPosition", EditConditionHides))
	bool bShowTail;

	UPROPERTY(EditAnywhere, meta = (Category = "Squad Position", EditCondition = "NodeType == EMetaGame_MapNodeType::SquadPosition", EditConditionHides))
	TArray<FVector> TransitionPoints;

	// UPROPERTY(BlueprintReadOnly, meta = (EditCondition = "false", EditConditionHides))
	// bool bIsMinorNode;
	//
	// UPROPERTY(BlueprintReadOnly, meta = (EditCondition = "false", EditConditionHides))
	// int TurnIndex;
};
