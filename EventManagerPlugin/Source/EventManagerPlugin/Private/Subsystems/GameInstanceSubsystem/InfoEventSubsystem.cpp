/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "InfoEventSubsystem.h"
#include "LogEventManagerPlugin.h"


// Info

const TSet<FName>& UInfoEventSubsystem::GetInfo() const {
	return Info;
}

bool UInfoEventSubsystem::GetHasFullInfo() const {
	return bHasFullInfo;
}

void UInfoEventSubsystem::SetHasFullInfo(bool bInHasFullInfo) {
	bHasFullInfo = bInHasFullInfo;
	OnHasFullInfoChanged.Broadcast(GetHasFullInfo());

	UE_LOG(LogEventManagerPlugin, Verbose, TEXT("SetHasFullInfo: %s. Add / Remove of info broadcasts when bHasFullInfo. Full info is required for intended info response."), (GetHasFullInfo() ? TEXT("true") : TEXT("false")));

	// This is only broadcasted when the full set of initial info has been loaded.
	// Otherwise someone responding would act on incomplete total information.
	if (GetHasFullInfo()) {
		for (const FName& InfoX : AddInfoBroadcastCue) {
			OnAddedInfo.Broadcast(InfoX);
		}
		AddInfoBroadcastCue.Empty();

		for (const FName& InfoX : RemoveInfoBroadcastCue) {
			OnRemovedInfo.Broadcast(InfoX);
		}
		RemoveInfoBroadcastCue.Empty();
	}

	/**
	* If info is not "full", we have loaded some info but not yet a complete set.
	* This is commonly an initial set of info, which is currently loading from a save file or elsewhere.
	*
	* Users should respond to addition / removal only when all initial info is present.
	* This is important because lack of info could lead to the following situation:
	* - One could start setting up a level in response to info "quest 1 added", not yet having info "quest 1 complete". The result could be incorrectly restarting quest 1.
	* Here are more examples how a combination of info is combined to get to desired results:
	* -	Quest 1 spawns apples if:
	* -- has info: "quest 1 started"
	* -- not info: "quest 1 apples spawned"
	* - Person A talks about the weather if:
	* -- has info: "weather: good"
	* -- has info: "player reputation: neutral"
	* -- not info: "time: night"
	* - Door X is open while:
	* -- has info: "key: X"
	* -- not info: "player reputation: enemy"
	*
	* When you require adding or removing info before the info is considered "full", then you should set bHasFullInfo = false at any moment. Do this with care!
	* While info is "full", you should set bHasFullInfo = true.
	* AddInfo and RemoveInfo will broadcast when bHasFullInfo.
	*/
}

void UInfoEventSubsystem::AddInfo(const FName& InInfo) {
	Info.Add(InInfo);
	if (GetHasFullInfo()) {
		// This is only broadcasted when the full set of initial info has been loaded.
		// Otherwise someone responding would act on incomplete total information.
		OnAddedInfo.Broadcast(InInfo);
	}
	else {
		AddInfoBroadcastCue.Add(InInfo);
	}
}

void UInfoEventSubsystem::RemoveInfo(const FName& InInfo) {
	Info.Remove(InInfo);
	if (GetHasFullInfo()) {
		// This is only broadcasted when the full set of initial info has been loaded.
		// Otherwise someone responding would act on incomplete total information.
		OnRemovedInfo.Broadcast(InInfo);
	}
	else {
		RemoveInfoBroadcastCue.Add(InInfo);
	}
}

bool UInfoEventSubsystem::FullInfoContains(const FName& InInfo) const {
	if (!GetHasFullInfo()) {
		UE_LOG(LogEventManagerPlugin, Error, TEXT("Asked if full info contains InInfo, but the info is not full. Returning false."));
		return false;
	}

	return GetInfo().Contains(InInfo);
}