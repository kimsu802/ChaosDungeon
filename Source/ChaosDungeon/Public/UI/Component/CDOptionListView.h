// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonListView.h"
#include "CDOptionListView.generated.h"

/**
 * 
 */
UCLASS()
class CHAOSDUNGEON_API UCDOptionListView : public UCommonListView
{
	GENERATED_BODY()

protected:
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Option")
	TMap<TSubclassOf<class UCDOptionData>, TSubclassOf<class UUserWidget>> entryClasses;

};
