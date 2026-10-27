UE 5.8 target compatibility update

Extract this ZIP into your existing FrontlineCommand project folder (the folder
containing FrontlineCommand.uproject), replacing the three Source/*.Target.cs files.
Then run Start-Cedar-Hollow.cmd again.

The editor, game and server targets now select BuildSettingsVersion.V7 and
EngineIncludeOrderVersion.Unreal5_8, matching UE 5.8 defaults. This removes the
shared UnrealEditor build-settings conflict reported by UnrealBuildTool.
No BuildEnvironment.Unique or bOverrideBuildEnvironment workaround is used.

This fixes target configuration; Unreal C++ compilation and asset generation
still require validation with your installed Unreal Engine on Windows.
