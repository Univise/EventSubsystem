/* Copyright (2017) Dominik Peacock. All Rights Reserved. */

#include "EventSubsystemPrivatePCH.h"

bool operator==(const FEventHandler& One, const FEventHandler& Two)
{
	return One.Function == Two.Function && One.This == Two.This;
}

UEventManager::UEventManager()
{
	/* Initialise MappedEvents to contain all events in existence*/
	for (TObjectIterator<UScriptStruct> It; It; ++It)
	{
		if (It->IsChildOf(FEventBase::StaticStruct()))
		{
			FEventSection& Section = MappedEvents.Add(*It);
			Section.EventHandlersWithPriorities.Add(FDummy()); // Priority Lowest
			Section.EventHandlersWithPriorities.Add(FDummy()); // Priority Low
			Section.EventHandlersWithPriorities.Add(FDummy()); // Priority Normal
			Section.EventHandlersWithPriorities.Add(FDummy()); // Priority High
			Section.EventHandlersWithPriorities.Add(FDummy()); // Priority Highest
			Section.EventHandlersWithPriorities.Add(FDummy()); // Priority Monitor
		}
	}
}

class UEventManager* UEventManager::GetInstance()
{
	return GetMutableDefault<UEventManager>();
}

TEnumAsByte<EEventRegistrationResult::Type> UEventManager::RegisterEvents(class UObject* Object, TEnumAsByte<EListenerPriority::Type> Priority)
{
	if (Object == NULL)
	{
		return EEventRegistrationResult::InvalidInput;
	}

	TArray<FEventHandler> EventHandlerList;
	ProduceHandlersFrom(Object, FEventBase::StaticStruct(), EventHandlerList);

	if (EventHandlerList.Num() == 0)
	{
		return EEventRegistrationResult::NoHandlersFound;
	}
	return AddHandlers(EventHandlerList, Priority);
}

TEnumAsByte<EEventRegistrationResult::Type> UEventManager::RegisterSingleEvent(class UObject* Object, class UScriptStruct* EventClass, TEnumAsByte<EListenerPriority::Type> Priority)
{
	if (Object == NULL)
	{
		return EEventRegistrationResult::InvalidInput;
	}

	TArray<FEventHandler> EventHandlerList;
	ProduceHandlersFrom(Object, EventClass, EventHandlerList, true);

	if (EventHandlerList.Num() == 0)
	{
		return EEventRegistrationResult::NoHandlersFound;
	}
	return AddHandlers(EventHandlerList, Priority);
}

void UEventManager::UnregisterEvents(class UObject* Object)
{
	if (Object == NULL)
	{
		return;
	}

	TArray<FEventHandler> EventHandlerList;
	ProduceHandlersFrom(Object, FEventBase::StaticStruct(), EventHandlerList);

	if (EventHandlerList.Num() == 0)
	{
		return;
	}
	RemoveHandlers(EventHandlerList);
}

void UEventManager::UnregisterSingleEvent(class UObject* Object, class UScriptStruct* EventClass)
{
	if (Object == NULL)
	{
		return;
	}

	TArray<FEventHandler> EventHandlerList;
	ProduceHandlersFrom(Object, EventClass, EventHandlerList);

	if (EventHandlerList.Num() == 0)
	{
		return;
	}
	RemoveHandlers(EventHandlerList);
}

void UEventManager::CallEvent(FEventBase& EventToCall, class UScriptStruct* EventClass)
{
	if (EventClass == NULL || EventClass == FEventBase::StaticStruct())
	{
		return;
	}
	if (!EventClass->IsChildOf(FEventBase::StaticStruct()))
	{
		return;
	}

#if SUPPORT_MUTLI_THREADING
	/* Wait for modifying threads to finish. */
	while (NumberOfModifiers.GetValue() > 0)
	{
	}
	NumberOfAccessors.Increment();
#endif // #if SUPPORT_MUTLI_THREADING

	/* Loop through all events left. */
	while (EventClass != FEventBase::StaticStruct())
	{
		FEventSection* Section = MappedEvents.Find(EventClass);
		/* Execute each event handler starting in the lowest priority; this way higher priority handlers get the last say on the event. */
		for (size_t j = 0; j < Section->EventHandlersWithPriorities.Num(); ++j)
		{
			/* Execute every event handler in the currenty priority. */
			for (size_t k = 0; k < Section->EventHandlersWithPriorities[j].Dummy.Num(); ++k)
			{
				Section->EventHandlersWithPriorities[j].Dummy[k].This->ProcessEvent(Section->EventHandlersWithPriorities[j].Dummy[k].Function, &EventToCall);
			}
		}
		/* Proceed with super class. */
		EventClass = Cast<UScriptStruct>(EventClass->GetSuperStruct());
	}

#if SUPPORT_MUTLI_THREADING
	NumberOfAccessors.Decrement();
#endif // #if SUPPORT_MUTLI_THREADING
}

bool UEventManager::CallEventSafely(FEventBase& EventToCall, class UScriptStruct* EventClass)
{
	UEventManager* Manager = UEventManager::GetInstance();
	if (Manager == NULL)
	{
		return false;
	}
	Manager->CallEvent(EventToCall, EventClass);
	return true;
}

void UEventManager::ProduceHandlersFrom(class UObject* Object, class UScriptStruct* EventClass, TArray<FEventHandler>& Result, bool bExact)
{
	UFunction* Func = NULL;
	UStructProperty* FirstParam = NULL;
	for (TFieldIterator<UFunction> FuncIt(Object->GetClass()); FuncIt; ++FuncIt)
	{
		Func = *FuncIt;
		/* 1st criteria: Allow only one parameter. */
		if (Func->NumParms == 1)
		{
			TFieldIterator<UProperty> ParamIt(Func);
			FirstParam = Cast<UStructProperty>(*ParamIt);
			/* 2nd criteria: First parameter must be a child of FEventBase. */
			if (FirstParam != NULL 
				&& (bExact || FirstParam->Struct->IsChildOf(EventClass))
				&& (!bExact || FirstParam->Struct == EventClass))
			{
				Result.Add(FEventHandler(Func, Object));
			}
		}
	}
}

TEnumAsByte<EEventRegistrationResult::Type> UEventManager::AddHandlers(TArray<FEventHandler>& HandlersToAdd, EListenerPriority::Type Priority)
{
#if SUPPORT_MUTLI_THREADING
	/* Tell reading threads a writing thread is waiting; prevents new reading threads from reading. */
	NumberOfModifiers.Increment();
	/* Waiting for reading threads to finish */
	while (NumberOfAccessors.GetValue() > 0)
	{
	}

	MapMutex.Lock();
#endif // #if SUPPORT_MUTLI_THREADING

	for (size_t i = 0; i < HandlersToAdd.Num(); ++i)
	{
		UScriptStruct* ScriptClass = Cast<UStructProperty>(*TFieldIterator<UProperty>(HandlersToAdd[i].Function))->Struct;
		FEventSection* Section = MappedEvents.Find(ScriptClass);
		Section->EventHandlersWithPriorities[Priority].Dummy.AddUnique(HandlersToAdd[i]);
	}

#if SUPPORT_MUTLI_THREADING
	/* Release lock */
	MapMutex.Unlock();
	/* Must tell reading threads that there is a writing thread less. */
	NumberOfModifiers.Decrement();
#endif

	return EEventRegistrationResult::Success;
}

void UEventManager::RemoveHandlers(TArray<FEventHandler>& HandlersToRemove)
{
#if SUPPORT_MUTLI_THREADING
	/* Tell reading threads a writing thread is waiting; prevents new reading threads from reading. */
	NumberOfModifiers.Increment();
	/* Waiting for reading threads to finish */
	while (NumberOfAccessors.GetValue() > 0)
	{
	}

	/* Aquire lock so only one writing thread can update the map. */
	MapMutex.Lock();
#endif // #if SUPPORT_MUTLI_THREADING

	for (size_t i = 0; i < HandlersToRemove.Num(); ++i)
	{
		UScriptStruct* ScriptClass = Cast<UStructProperty>(*TFieldIterator<UProperty>(HandlersToRemove[i].Function))->Struct;
		FEventSection* Section = MappedEvents.Find(ScriptClass);
		for (size_t k = 0; k < Section->EventHandlersWithPriorities.Num(); ++k)
		{
			/* There can only be one handler in the entire array EventHandlersWithPriorities. */
			if (Section->EventHandlersWithPriorities[k].Dummy.RemoveSingle(HandlersToRemove[i]) > 0)
			{
				break;
			}
		}
	}

#if SUPPORT_MUTLI_THREADING
	/* Release lock */
	MapMutex.Unlock();
	/* Must tell reading threads that there is a writing thread less. */
	NumberOfModifiers.Decrement();
#endif // #if SUPPORT_MUTLI_THREADING
}