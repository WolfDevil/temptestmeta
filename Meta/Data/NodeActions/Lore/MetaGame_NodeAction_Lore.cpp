#include "MetaGame_NodeAction_Lore.h"

#include "T01/Core/Subsystem/Meta/MetaGameSubsystem.h"

void UMetaGame_NodeAction_Lore::ExecuteNodeAction(UObject* Context)
{
	//Show UI
	UMetaGameSubsystem* MetaGame = Cast<UMetaGameSubsystem>(Context);
	if (!MetaGame) return;
	if (!DataAsset) return;

	//TODO: META GAME: Some more conditions, like if activity already started resolving (if we are going to have delayed resolve)

	bool WidgetSpawned = false;
	MetaGame->ShowLoreUIFromNode(NodeID, WidgetSpawned);
}
