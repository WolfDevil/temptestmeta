#pragma once
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_Base.h"
#include "MetaGame_NodeAction_Lore.generated.h"

UCLASS(BlueprintType)
class UMetaGame_NodeAction_Lore : public UMetaGame_NodeAction_Base
{
	GENERATED_BODY()

public:
	virtual void ExecuteNodeAction(UObject* Context) override;
};
