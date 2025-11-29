#pragma once
#include "EMetaGame_MapNodeState.generated.h"

UENUM(BlueprintType)
enum class EMetaGame_MapNodeState : uint8
{
	Undefined = 0,
	Locked,
	Unlocked,
	Pending,
	Completed
};
