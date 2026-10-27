using UnrealBuildTool;
public class FrontlineCommandServerTarget : TargetRules {
 public FrontlineCommandServerTarget(TargetInfo Target) : base(Target) {
  Type=TargetType.Server; DefaultBuildSettings=BuildSettingsVersion.V7;
  IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_8; ExtraModuleNames.Add("FrontlineCommand");
 }
}
