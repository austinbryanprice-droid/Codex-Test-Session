using UnrealBuildTool;
public class FrontlineCommand : ModuleRules {
 public FrontlineCommand(ReadOnlyTargetRules Target) : base(Target) {
  PublicIncludePaths.Add(ModuleDirectory);
  PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs; CppStandard=CppStandardVersion.Cpp20;
  PublicDependencyModuleNames.AddRange(new [] {"Core","CoreUObject","Engine","InputCore","UMG","Slate","SlateCore","CommonUI","NavigationSystem","AIModule"});
 }
}
