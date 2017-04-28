// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EventSubsystem : ModuleRules
{
	public EventSubsystem(TargetInfo Target)
	{
		
		PublicIncludePaths.AddRange(
			new string[] {
				"EventSubsystem/Public",
                "EventSubsystem/Public/Communication",
                "EventSubsystem/Public/Events",
                "EventSubsystem/Public/Events/Entity"
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"EventSubsystem/Private",
                "EventSubsystem/Private/Communication",
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine"
			}
			);
	}
}
