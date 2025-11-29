#pragma once
#include "CommonUserWidget.h"
#include "T01/Core/Subsystem/Meta/Data/MetaGame_RewardNotificationData.h"

#include "BaseMetaActivityRewardsWidget.generated.h"

UCLASS(BlueprintType)
class UBaseMetaActivityRewardsWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
#pragma warning( push )
#pragma warning( disable: 4263 4264 ) // intentionally function with same name, but different goal
	void Initialize(FMetaGame_RewardNotificationData NotificationData);
};
#pragma warning( pop )