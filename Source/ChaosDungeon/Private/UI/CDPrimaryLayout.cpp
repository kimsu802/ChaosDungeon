#include "UI/CDPrimaryLayout.h"
#include "CommonActivatableWidget.h"
#include "Core/CDGameplayTags.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UCDPrimaryLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	layers.Add(CDTags::UI_Layer_Game, gameLayer);
	layers.Add(CDTags::UI_Layer_GameMenu, gameMenuLayer);
	layers.Add(CDTags::UI_Layer_Modal, modalLayer);
}

UCommonActivatableWidget* UCDPrimaryLayout::PushToLayer(FGameplayTag layerTag, TSubclassOf<UCommonActivatableWidget> screenClass)
{
	UCommonActivatableWidgetStack* layer = layers.FindRef(layerTag);
	if (!layer || !screenClass)
	{
		return nullptr;
	}
	return layer->AddWidget(screenClass);
}

bool UCDPrimaryLayout::DeactivateIfActive(FGameplayTag layerTag, TSubclassOf<UCommonActivatableWidget> screenClass)
{
	const UCommonActivatableWidgetStack* layer = layers.FindRef(layerTag);
	UCommonActivatableWidget* active = layer ? layer->GetActiveWidget() : nullptr;
	if (active && screenClass && active->IsA(screenClass))
	{
		active->DeactivateWidget();
		return true;
	}
	return false;
}

void UCDPrimaryLayout::ClearStack(FGameplayTag layerTag)
{
	if (UCommonActivatableWidgetStack* layer = layers.FindRef(layerTag))
	{
		layer->ClearWidgets();
	}
}
