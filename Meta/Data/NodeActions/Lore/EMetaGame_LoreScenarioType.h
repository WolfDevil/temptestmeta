#pragma once
#include "EMetaGame_LoreScenarioType.generated.h"

UENUM(BlueprintType)
enum class EMetaGame_LoreScenarioType : uint8
{
	Undefined = 0,
	Continue,
	TakeResources,
	LockUnit
};
