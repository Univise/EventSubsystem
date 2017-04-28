// Intellectual property (2016) of Blake Univise a.k.a. Dominik Peacock. All Rights Reserved.

#pragma once 

#include "Events/EventBase.h"
#include "EntityDeathEvent.generated.h"

/**
 * Called when an entity (usually player) dies.
 *
 * Informing event.
 */
USTRUCT(BlueprintType)
struct FEntityDeathEvent : public FEventBase
{
	GENERATED_BODY()
public:

	/**
	 * The controller that owned the now dead pawn.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Entity Death Event")
	class AController* Controller;

	/**
	 * The pawn which died.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Entity Death Event")
	class APawn* DeadPawn;

public:

	FEntityDeathEvent(class AController* MyController = NULL, class APawn* MyDeadPawn = NULL)
		:
		Controller(MyController),
		DeadPawn(MyDeadPawn)
	{}
};
