#pragma once
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_Base.h"

#include "MetaGame_NodeAction_Mission.generated.h"

UCLASS()
class T01_API UMetaGame_NodeAction_Mission : public UMetaGame_NodeAction_Base
{
	GENERATED_BODY()

public:
	virtual void ExecuteNodeAction(UObject* Context) override;
};
