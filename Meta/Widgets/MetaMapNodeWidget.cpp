// Fill out your copyright notice in the Description page of Project Settings.


#include "MetaMapNodeWidget.h"

#include "T01/Core/Subsystem/Meta/Data/NodeActions/Base/MetaGame_NodeAction_BaseData.h"

void UMetaMapNodeWidget::HandleButtonClicked()
{
	OnNodeClicked.Broadcast(NodeData.ID);
	OnNodeClickedDynamic.Broadcast(NodeData.ID);
}
