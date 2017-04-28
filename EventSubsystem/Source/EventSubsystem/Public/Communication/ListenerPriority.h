// Intellectual property (2016) of Blake Univise a.k.a. Dominik Peacock. All Rights Reserved.

#pragma once

#include "ListenerPriority.generated.h"

/**
 * A listener's priority dictates how important an a listener's modifications to an event are.
 * Higher priority listeners get the last say in an event's state.
 */
UENUM(BlueprintType)
namespace EListenerPriority
{
	enum Type
	{
		Lowest UMETA(DisplayName = "Lowest"),
		Low UMETA(DisplayName = "Lowest"),
		Normal UMETA(DisplayName = "Lowest"),
		High UMETA(DisplayName = "Lowest"),
		Highest UMETA(DisplayName = "Lowest"),

		/**
		 * Listeners with the monitor priority may not modify an event's state, it is strictly for monitoring 
		 * an event's outcome.
		 */
		Monitor UMETA(DisplayName = "Lowest")
	};
}