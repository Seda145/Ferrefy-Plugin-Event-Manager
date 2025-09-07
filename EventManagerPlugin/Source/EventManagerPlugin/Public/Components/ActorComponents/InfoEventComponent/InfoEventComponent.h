/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "InfoEventComponent.generated.h"


class UInfoEventSubsystem;


DECLARE_DYNAMIC_DELEGATE(FOnInfoEvent);


USTRUCT(BlueprintType)
struct EVENTMANAGERPLUGIN_API FS_InfoEventTrigger {
	GENERATED_BODY()

	/**
	* An event triggers when the info subsystem holds a combination of HasInfo and not NotInfo.
	* This can look like: HasInfo(mission_1_starts) NotInfo(spawned_mission_1_enemies),
	* Where the event on example class EnemySpawner looks like: SpawnEnemies() AddInfo(spawned_mission_1_enemies)
	* To summarize: The SpawnEnemies event was triggered on EnemySpawner conditionally, which spawned the enemies and added info required to stop a future retrigger of the event. 
	* 
	* In general it is better to add info working as "NotInfo", than to remove info, to avoid a trigger condition.
	* This depends a bit on the situation. A weather system reporting the current weather as info might remove "weather_sunny" and add "weather_rain".
	* A game mission however, often acts a bit as a log of things which did happen and can not be undone. ("started_mission_1", "opened_door_1", "found_key_x", ...)
	* 
	* A retrigger is guarded against, as long as the event was triggerable on the previous check (when info was added or removed globally).
	* An event trigger meeting info conditions will trigger the event only once until the conditions are no longer met. then it can retrigger when info conditions are met again.
	* This can reduce the amount of HasInfo / NotInfo required, has HasInfo(found_key) does not require NotInfo(opened_door) to prevent a retrigger.
	* However, This depends entirely on your implementation, as it serves more as a basic guard. 
	* If for example, you use a save game system to load your mission's previously spawned enemies, nothing will prevent the event on HasInfo(mission_started) to spawn duplicates if you have not guarded against this with NotInfo(spawned_enemies).
	*/

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info Event Trigger")
		TSet<FName> HasInfo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info Event Trigger")
		TSet<FName> NotInfo;

	/* Leave false. This property is managed by the event system. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info Event Trigger")
		bool bIsGuardingRetrigger;

	// Initialize
	FS_InfoEventTrigger()
		: HasInfo (TSet<FName>())
		, NotInfo (TSet<FName>())
		, bIsGuardingRetrigger(false)
	{}
	FS_InfoEventTrigger(const TSet<FName>& InHasInfo, const TSet<FName>& InNotInfo, bool bInIsGuardingRetrigger)
		: HasInfo (InHasInfo)
		, NotInfo (InNotInfo)
		, bIsGuardingRetrigger(bInIsGuardingRetrigger)
	{}
};


USTRUCT(BlueprintType)
struct EVENTMANAGERPLUGIN_API FS_InfoEvent {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info Event")
		FS_InfoEventTrigger EventTrigger;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Info Event")
		FOnInfoEvent Delegate;

	// Initialize
	FS_InfoEvent()
		: EventTrigger (FS_InfoEventTrigger())
		, Delegate (FOnInfoEvent())
	{}
	FS_InfoEvent(const FS_InfoEventTrigger& InEventTrigger, const FOnInfoEvent& InDelegate)
		: EventTrigger (InEventTrigger)
		, Delegate (InDelegate)
	{}
};


/**
* The Info Event Component listens for info from the Info Event Subsystem.
* It can register "Info Events", which are methods called automatically under configured conditions (Info Event Subsystem has info X, not info Y).
*/
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent))
class EVENTMANAGERPLUGIN_API UInfoEventComponent : public UActorComponent {
	GENERATED_BODY()

private:

	// InfoEvents

	UPROPERTY(Transient)
		UInfoEventSubsystem* InfoEventSubsystem = nullptr;

	// Info

	UPROPERTY(Transient)
		TArray<FS_InfoEvent> InfoEvents;

protected:

public:

private:

	// Info

	const TArray<FS_InfoEvent>& GetInfoEvents() const;

	void RespondToInfo();

	// Delegates | Info

	UFUNCTION()
		void ActOnAddedInfo(const FName& InInfo);

	UFUNCTION()
		void ActOnRemovedInfo(const FName& InInfo);

protected:

public:

	// Setup

	virtual void BeginPlay() override;


	/* Function for Blueprints, to add an info event. C++ users are more likely to use "BindInfoEvent". */
	UFUNCTION(BlueprintCallable, Category = "Info")
		void AddInfoEvent(const FS_InfoEvent& InInfoEvent);

	/* Utility function for Blueprints, to add multiple info events at once. C++ users are more likely to use "BindInfoEvent". */
	UFUNCTION(BlueprintCallable, Category = "Info")
		void AddInfoEvents(const TArray<FS_InfoEvent>& InInfoEvents);

	// Utility

	/**
	* Utility method to create + bind a delegate and pass it on. Binds a UObject method address on this UObject instance to this delegate.
	*
	* @param	InObject			UObject instance
	* @param	InFunc			    Member function address pointer
	* @param	InFunctionName		Name of member function, without class name
	*
	* NOTE:  Do not call this function directly.  Instead, call BindInfoEvent() which is a macro proxy function that
	*        automatically sets the function name string for the caller.
	*/
	template<typename UserClass>
	void __Internal_BindInfoEvent(const FS_InfoEventTrigger& InInfoTrigger, UserClass* InObject, typename TMemFunPtrType<false, UserClass, void()>::Type InFunc, const FName& InFuncName) {
		FOnInfoEvent Callback;
		Callback.__Internal_BindDynamic(InObject, InFunc, InFuncName);
		FS_InfoEvent InfoEvent = FS_InfoEvent(InInfoTrigger, Callback);
		InfoEvents.Add(InfoEvent);
	}

	/**
	* Utility method to create + bind a delegate and pass it on. Helper macro to bind a member UFUNCTION on this UObject instance to a dynamic delegate.
	*
	* @param	InObject	UObject instance
	* @param	InFunc		Function pointer to member UFUNCTION, usually in form &UClassName::FunctionName
	*/
#define BindInfoEvent(InInfoTrigger, InObject, InFunc) __Internal_BindInfoEvent(InInfoTrigger, InObject, InFunc, STATIC_FUNCTION_FNAME(TEXT(#InFunc)))

};
