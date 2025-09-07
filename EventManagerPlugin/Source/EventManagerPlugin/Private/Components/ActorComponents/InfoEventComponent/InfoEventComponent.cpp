/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "InfoEventComponent.h"
#include "InfoEventSubsystem.h"
#include "LogEventManagerPlugin.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"


// Setup

void UInfoEventComponent::BeginPlay() {
	Super::BeginPlay();

	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
	InfoEventSubsystem = GameInstance != nullptr ? GameInstance->GetSubsystem<UInfoEventSubsystem>() : nullptr;
	check(IsValid(InfoEventSubsystem));

	InfoEventSubsystem->OnAddedInfo.AddDynamic(this, &UInfoEventComponent::ActOnAddedInfo);
	InfoEventSubsystem->OnRemovedInfo.AddDynamic(this, &UInfoEventComponent::ActOnRemovedInfo);
	UE_LOG(LogEventManagerPlugin, VeryVerbose, TEXT("%s is now set up to respond to info from the InfoEventSubsystem."), *GetName());

	// Respond immediately if initial info is already loaded.
	// Else RespondToInfo will be called later when the InfoEventSubsystem reports full info and processes its add / remove broadcast cue.
	if (InfoEventSubsystem->GetHasFullInfo()) {
		RespondToInfo();
	}
}

// Info

const TArray<FS_InfoEvent>& UInfoEventComponent::GetInfoEvents() const {
	return InfoEvents;
}

void UInfoEventComponent::AddInfoEvent(const FS_InfoEvent& InInfoEvent) {
	InfoEvents.Add(InInfoEvent);
}

void UInfoEventComponent::AddInfoEvents(const TArray<FS_InfoEvent>& InInfoEvents) {
	for (const FS_InfoEvent& EventX : InInfoEvents) {
		InfoEvents.Add(EventX);
	}
}

void UInfoEventComponent::RespondToInfo() {
	if (!IsValid(InfoEventSubsystem)) {
		UE_LOG(LogEventManagerPlugin, Error, TEXT("InfoEventSubsystem invalid, can't attempt events."));
		return;
	}
	// It is wrong to act on info when only part of the info is available.
	check(InfoEventSubsystem->GetHasFullInfo());

	for (FS_InfoEvent& InfoEventX : InfoEvents) {
		bool bCanAttempt = true;
		for (const FName& InfoX : InfoEventX.EventTrigger.HasInfo) {
			if (!InfoEventSubsystem->FullInfoContains(InfoX)) {
				bCanAttempt = false;
				break;
			}
		}
		if (bCanAttempt) {
			for (const FName& InfoX : InfoEventX.EventTrigger.NotInfo) {
				if (InfoEventSubsystem->FullInfoContains(InfoX)) {
					bCanAttempt = false;
					break;
				}
			}
		}
		if (bCanAttempt) {
			if (!InfoEventX.EventTrigger.bIsGuardingRetrigger) {
				UE_LOG(LogEventManagerPlugin, Verbose, TEXT("InfoEvent triggering: %s"), *InfoEventX.Delegate.GetFunctionName().ToString());
				InfoEventX.Delegate.ExecuteIfBound();
				InfoEventX.EventTrigger.bIsGuardingRetrigger = true;
			}
		}
		else {
			InfoEventX.EventTrigger.bIsGuardingRetrigger = false;
		}
	}
}

// Delegates | Info

void UInfoEventComponent::ActOnAddedInfo(const FName& InInfo) {
	RespondToInfo();
}

void UInfoEventComponent::ActOnRemovedInfo(const FName& InInfo) {
	RespondToInfo();
}