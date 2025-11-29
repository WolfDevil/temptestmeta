#include "BaseMetaMissionWidget.h"


void UBaseMetaMissionWidget::Initialize_Implementation(UMetaGame_NodeAction_Base* InNodeAction)
{
	NodeAction = InNodeAction;
}

void UBaseMetaMissionWidget::InitializeWithFighters_Implementation(UMetaGame_NodeAction_Base* InNodeAction, const TArray<FMetaGame_FighterData>& Fighters, const TArray<FMetaGame_FighterData>& ForcedFighters)
{
	NodeAction = InNodeAction;
}
