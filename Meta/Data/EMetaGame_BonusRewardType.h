#pragma once
#include "EMetaGame_BonusRewardType.generated.h"

UENUM(BlueprintType)
enum class EMetaGame_BonusRewardType : uint8
{
	Undefined = 0,
	AllThreatsClosed,
	AdditionalResources,
};
