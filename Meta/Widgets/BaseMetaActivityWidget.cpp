// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMetaActivityWidget.h"

void UBaseMetaActivityWidget::Initialize_Implementation(UMetaGame_NodeAction_Base* InNodeAction)
{
	NodeAction = InNodeAction;
}

void UBaseMetaActivityWidget::InitializeWithFighters_Implementation(UMetaGame_NodeAction_Base* InNodeAction, const TArray<FMetaGame_FighterData>& Fighters)
{
	NodeAction = InNodeAction;
}
