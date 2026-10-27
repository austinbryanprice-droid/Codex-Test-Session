using UnrealBuildTool;
public class FrontlineCommandEditorTarget : TargetRules {
 public FrontlineCommandEditorTarget(TargetInfo Target) : base(Target) {
  Type=TargetType.Editor; DefaultBuildSettings=BuildSettingsVersion.V7;
  IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_8; ExtraModuleNames.Add("FrontlineCommand");
 }
}
