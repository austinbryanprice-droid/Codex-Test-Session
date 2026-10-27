using UnrealBuildTool;
public class FrontlineCommandServerTarget : TargetRules {
 public FrontlineCommandServerTarget(TargetInfo Target) : base(Target) {
  Type=TargetType.Server; DefaultBuildSettings=BuildSettingsVersion.V5;
  IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_5; ExtraModuleNames.Add("FrontlineCommand");
 }
}
