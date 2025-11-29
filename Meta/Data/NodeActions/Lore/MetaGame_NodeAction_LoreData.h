#pragma once
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_BaseData.h"

#include "MetaGame_NodeAction_LoreData.generated.h"

UCLASS()
class T01_API UMetaGame_NodeAction_LoreData : public UMetaGame_NodeAction_BaseData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Category = "Lore"))
	FName LoreID;
};
