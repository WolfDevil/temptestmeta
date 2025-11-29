#pragma once
#include "EMetaGame_NextTurnStatus.generated.h"

UENUM(BlueprintType)
enum class EMetaGame_NextTurnStatus : uint8
{
	NotAssignedMission = 0,
	NotAssignedActivities = 1,
	NoMoreTurns = 2,
	InternalError = 3,
	
	AllGood = 10,
};
