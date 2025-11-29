#pragma once
#include "CommonUserWidget.h"
#include "T01/Core/Subsystem/Meta/Data/MetaGame_FighterData.h"
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_Base.h"
#include "BaseMetaMissionWidget.generated.h"

UCLASS(BlueprintType)
class UBaseMetaMissionWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Initialize(UMetaGame_NodeAction_Base* InNodeAction);

	UFUNCTION(BlueprintNativeEvent)
	void InitializeWithFighters(UMetaGame_NodeAction_Base* InNodeAction, const TArray<FMetaGame_FighterData>& Fighters, const TArray<FMetaGame_FighterData>& ForcedFighters);

protected:
	UPROPERTY(BlueprintReadWrite)
	UMetaGame_NodeAction_Base* NodeAction;
};
