using UnrealBuildTool;
public class FrontlineCommandEditorTarget : TargetRules {
 public FrontlineCommandEditorTarget(TargetInfo Target) : base(Target) {
  Type=TargetType.Editor; DefaultBuildSettings=BuildSettingsVersion.V5;
  IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_5; ExtraModuleNames.Add("FrontlineCommand");
 }
}
