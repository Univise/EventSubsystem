/* Copyright (2017) Dominik Peacock. All Rights Reserved. */

#pragma once 

#include "EventBase.generated.h"

/**
 * Base-class for all events.
 *
 * There are two types of events: Informing events and cancellable events. 
 *
 * Informing events simply tell an event listener that something has happened, for example a player has just died.
 *
 * Cancellable events are called at points in the game before an action is taken and they can be cancelled:
 * For instance, a PrePlayerDamageEvent is called before damage is applied to a player and if it is cancelled
 * no damage will be applied to the player. Events which can be cancelled will have a property bCancelled.
 *
 * Design note: It is not possible to have structs implement interfaces; this is the reason why cancellable events do not
 * simply implement an ICancellable interface but in fact must have a bCancelled property. The type of events will always
 * be known both to listeners and callers so this does not pose a problem.
 */
USTRUCT(BlueprintType)
struct FEventBase
{
	GENERATED_BODY()
public:

};