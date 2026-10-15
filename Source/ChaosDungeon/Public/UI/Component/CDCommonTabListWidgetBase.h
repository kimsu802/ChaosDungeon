// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "CDCommonTabListWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDCommonTabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

public:
	//void UMainMenuTabListWidgetBase::RequestRegisterTab(const FName& InTabID, const FText& InTabDisplayName)
	//{
	//	RegisterTab(InTabID, TabButtonEntryWidgetClass, nullptr);

	//	if (UMainMenuCommonButtonBase* FoundButton = Cast<UMainMenuCommonButtonBase>(GetTabButtonBaseByID(InTabID)))
	//	{
	//		FoundButton->SetButtonText(InTabDisplayName);
	//	}
	//}
	
};
