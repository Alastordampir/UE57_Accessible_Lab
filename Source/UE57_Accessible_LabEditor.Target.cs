using UnrealBuildTool;
using System.Collections.Generic;
public class UE57_Accessible_LabEditorTarget : TargetRules
{
    public UE57_Accessible_LabEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("UE57_Accessible_Lab");
    }
}