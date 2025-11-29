// Fill out your copyright notice in the Description page of Project Settings.


#include "MetaGame_NodeAction_Mission.h"

#include "MetaGame_NodeAction_MissionData.h"
#include "Kismet/GameplayStatics.h"
#include "T01/Core/Subsystem/Meta/MetaGameSubsystem.h"

void UMetaGame_NodeAction_Mission::ExecuteNodeAction(UObject* Context)
{
	//Show UI
	UMetaGameSubsystem* MetaGame = Cast<UMetaGameSubsystem>(Context);
	if (!MetaGame) return;
	if (!DataAsset) return;
	
	bool WidgetSpawned = false;
	MetaGame->ShowMissionUI(NodeID, WidgetSpawned);
}
