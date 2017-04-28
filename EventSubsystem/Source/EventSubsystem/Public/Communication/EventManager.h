/* Copyright (2017) Dominik Peacock. All Rights Reserved. */

#pragma once

#include "Components/ActorComponent.h"
#include "Communication/ListenerPriority.h"
#include "Events/EventBase.h"
#include "BuildConfiguration.h"
#include "EventManager.generated.h"

UENUM(BlueprintType)
namespace EEventRegistrationResult
{
	enum Type
	{
		/**
		 * Successfully registered event.
		 */
		Success UMETA(DisplayName = "Success"),
		/**
		 * Failed to register event because it was already registered.
		 */
		AlreadyRegistered UMETA(DisplayName = "AlreadyRegistered"),
		/**
		 * Failed to register event because no handler function was found (only for RegisterSingleEvent).
		 */
		FailedToFindHandler UMETA(DisplayName = "Failed to find Handler"),
		/**
		 * Failed to find at least one handler function (only for RegisterEvents)
		 */
		NoHandlersFound UMETA(DisplayName = "No handlers found"),
		/**
		 * The method was called with invalid arguments.
		 */
		InvalidInput UMETA(DisplayName = "Invalid input")
	};
}

USTRUCT()
struct FEventHandler
{
	GENERATED_BODY()
public:

	/**
	 * The function to call for a certain event.
	 */
	UPROPERTY()
	class UFunction* Function;

	/**
	 * The this pointer of the function.
	 */
	UPROPERTY()
	class UObject* This;

public:

	FEventHandler(class UFunction* MyFunction = NULL, class UObject* MyThis = NULL)
		:
		Function(MyFunction),
		This(MyThis)
	{}

	friend bool operator==(const FEventHandler& One, const FEventHandler& Two);
};

/**
 * Dummy structure for FEventSection because Unreal Header Tool does not allow TArray<TArray<>>.
 */
USTRUCT()
struct FDummy
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TArray<FEventHandler> Dummy;

};

/**
 * Stores information for event handling and thread-safely adding/removing event handlers.
 */
USTRUCT()
struct FEventSection
{
	GENERATED_BODY()
public:

	/**
	 * Every value of EListenerPriority is a valid index. At each index stores the event handlers,
	 * with the priority of the associated index.
	 */
	UPROPERTY()
	TArray<FDummy> EventHandlersWithPriorities;

};

/**
 * Performs event handling and registration.
 *
 * Compile this code with SUPPORT_MUTLI_THREADING set to 1 in BuildConfiguration.h to allow mutlithreading support (default is 0).
 * Registering / unregistering listeners blocks all events for the duration of modiyfing the listners mapping. There exists a more
 * efficient algorithm which will only blocks those events for which an object is registering, however, the trade-off between its
 * complexity and its efficientcy is very low assuming most objects register while the map is loaded.
 */
UCLASS(BlueprintType)
class EVENTSUBSYSTEM_API UEventManager : public UObject
{
	GENERATED_BODY()
public: /* Construction */

	UEventManager();

	/**
	 * @return Obtains the run-time event manager instance
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Event Manager")
	static class UEventManager* GetInstance();

public: /* Registration */

	/**
	 * Registers the object passed to receive events; all event handling functions will have
	 * the same priority as "Priority".
	 *
	 * This method is thread-safe. It waits for all events to finish execution and then register
	 * the object passed.
	 *
	 * This method will search the object's class for UFunctions whose signatures fulfill:
	 *			1. signature is a single parameter
	 *			2. the parameter's type derives from FEventBase
	 *	Additionally the parameter should be reference; if it wishes to modify events, it must be a reference.
	 *
	 * @param Object The object which is registering for events.
	 * @param Priority The priority to register all events with. Higher priority listeners will get a last
	 *					say with what happens to the outcome of an event.
	 *
	 * @return The result of the attempted registration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Event Manager")
	TEnumAsByte<EEventRegistrationResult::Type> RegisterEvents(class UObject* Object, TEnumAsByte<EListenerPriority::Type> Priority = EListenerPriority::Normal);

	/**
	 * Registers the object passed for the event identified by by "EventClass"; the event handling function will be given
	 * the priority passed.
	 *
	 * This method is thread-safe. It waits for all events to finish execution and then register
	 * the object passed.
	 *
	 * This method will search the object's class for the UFunction whose signature fulfills:
	 *			1. signature is a single parameter
	 *			2. the parameter's type derives from FEventBase
	 *			3. the parameter is a reference (const or non-const).
	 *
	 * @param Object The object which is registering for events.
	 * @param EventClass The class of the event to register for as obtained by calling the static function StaticStruct(); the class must match
	 *					exactly the handler function's event type (no sub-classes may be passed).
	 * @param Priority The priority to register all events with. Higher priority listeners will get a last
	 *					say with what happens to the outcome of an event.
	 *
	 * @return The result of the attempted registration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Event Manager")
	TEnumAsByte<EEventRegistrationResult::Type> RegisterSingleEvent(class UObject* Object, class UScriptStruct* EventClass, TEnumAsByte<EListenerPriority::Type> Priority = EListenerPriority::Normal);

	/**
	 * Unregisters all event handles associated with the passed object.
	 *
	 * This method is thread-safe. It waits for all events to finish execution and then register
	 * the object passed.
	 *
	 * @param Object The object to unregister events for.
	 */
	UFUNCTION(BlueprintCallable, Category = "Event Manager")
	void UnregisterEvents(class UObject* Object);

	/**
	 * Unregisters the event handler for the passed event class associated with the passed object.
	 *
	 * This method is thread-safe. It waits for all events to finish execution and then register
	 * the object passed.
	 *
	 * @param Object The object to unregister an event for
	 * @param EventClass The event to unregister.
	 */
	UFUNCTION(BlueprintCallable, Category = "Event Manager")
	void UnregisterSingleEvent(class UObject* Object, class UScriptStruct* EventClass);

public: /* Calling */

	 /**
	  * Calls an event.
	  *
	  * Useful for determining on how an action should be completed. For instance, if an object wishes to
	  * apply damage to another entity, it could call an event and registered listeners may perform their logic
	  * and whether to add to the damage to apply. After an event call is complete, the event caller can apply the
	  * that is now in the event structure.
	  *
	  * @param EventToCall The event to call; it may be modified.
	  * @param EventClass The result of calling StaticClass() on the type of the event.
	  */
	UFUNCTION(BlueprintCallable, Category = "Event Manager")
	void CallEvent(UPARAM(ref) FEventBase& EventToCall, class UScriptStruct* EventClass);

	/**
	 * Performs CallEvent only if on the server.
	 *
	 * @return True, if the event was called, false, if not. 
	 *
	 * @see CallEvent
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Event Manager")
	static bool CallEventSafely(UPARAM(ref) FEventBase& EventToCall, class UScriptStruct* EventClass);

private: /* Functions */

	void ProduceHandlersFrom(class UObject* Object, class UScriptStruct* EventClass, TArray<FEventHandler>& Result, bool bExact = false);

	/**
	 * Adds the event handlers specified.
	 */
	TEnumAsByte<EEventRegistrationResult::Type> AddHandlers(TArray<FEventHandler>& HandlersToAdd, EListenerPriority::Type Priority);

	/**
	 * Removes the event handlers specified.
	 */
	void RemoveHandlers(TArray<FEventHandler>& HandlersToRemove);

private: /* Event related properties */

	 /**
	 * Binds each event class registered so far to a set of handlers.
	 */
	UPROPERTY()
	TMap<class UScriptStruct*, FEventSection> MappedEvents;

#if SUPPORT_MUTLI_THREADING

	FThreadSafeCounter NumberOfAccessors;

	FThreadSafeCounter NumberOfModifiers;

	FCriticalSection MapMutex;

#endif // #if SUPPORT_MUTLI_THREADING

};