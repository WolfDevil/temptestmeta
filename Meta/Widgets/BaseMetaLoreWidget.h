#pragma once
#include "CommonUserWidget.h"
#include "T01/Core/Subsystem/Meta/Data/NodeActions/Lore/MetaGame_LoreData.h"
#include "BaseMetaLoreWidget.generated.h"

UCLASS(BlueprintType)
class UBaseMetaLoreWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

#pragma warning(push)
#pragma warning(disable: 4263 4264) // intentionally function with same name, but different goal
	UFUNCTION(BlueprintNativeEvent)
	void Initialize(FMetaGame_LoreData InData);
#pragma warning(pop)

protected:
	UPROPERTY(BlueprintReadWrite)
	FMetaGame_LoreData Data;

#pragma warning(push)
#pragma warning(disable: 4263 4264) 
};
#pragma warning(pop)
