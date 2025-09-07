/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "InfoEventSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInfoEventSubsystemHasFullInfoChanged, bool, bInHasFullInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInfoChanged, const FName&, InInfo);


/**
* 
*/
UCLASS()
class EVENTMANAGERPLUGIN_API UInfoEventSubsystem : public UGameInstanceSubsystem {
	GENERATED_BODY()

private:

	// Info

	/* 
	* bHasFullInfo should be set to false while you are loading an initial set of info, of which the total is required for the info to be considered "full" (complete). 
	* This prevents broadcasting of added / removed info while the total info is not complete.
	*/
	UPROPERTY(EditAnywhere, Category = "Info")
		bool bHasFullInfo = true;

	UPROPERTY(Transient)
		TSet<FName> Info;

	UPROPERTY(Transient)
		TSet<FName> AddInfoBroadcastCue;

	UPROPERTY(Transient)
		TSet<FName> RemoveInfoBroadcastCue;

protected:

public:

	// Delegates | Info

	/* Called after Initial info has been loaded. */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Info")
	FOnInfoEventSubsystemHasFullInfoChanged OnHasFullInfoChanged;

	/* Called when a new info has been added, but only after all Initial info has been loaded. This ensures you act on a fully loaded info collection. */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Info")
		FOnInfoChanged OnAddedInfo;

	/* Called when an info has been removed, but only after all Initial info has been loaded. This ensures you act on a fully loaded info collection. */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Info")
		FOnInfoChanged OnRemovedInfo;

private:

	const TSet<FName>& GetInfo() const;

protected:

public:

	// Info

	/* 
	* bHasFullInfo should be set to false while you are loading an initial set of info, of which the total is required for the info to be considered "full" (complete). 
	* This prevents broadcasting of added / removed info while the total info is not complete.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Info")
		bool GetHasFullInfo() const;

	/* 
	* bHasFullInfo should be set to false while you are loading an initial set of info, of which the total is required for the info to be considered "full" (complete). 
	* This prevents broadcasting of added / removed info while the total info is not complete.
	*/
	UFUNCTION(BlueprintCallable, Category = "Info")
		void SetHasFullInfo(bool bInHasFullInfo);

	/* Adds info and immediately broadcasts if there is full info. Otherwise broadcasts when info is full. */
	UFUNCTION(BlueprintCallable, Category = "Info")
		void AddInfo(const FName& InInfo);

	/* Adds info and immediately broadcasts if there is full info. Otherwise broadcasts when info is full. */
	UFUNCTION(BlueprintCallable, Category = "Info")
		void RemoveInfo(const FName& InInfo);

	/* Returns true if the full set of info contains InInfo. Errors and returns false if the info is not full. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Info")
		bool FullInfoContains(const FName& InInfo) const ;

};
