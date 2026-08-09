using UnrealBuildTool;
using System.Collections.Generic;
public class UE57_Accessible_LabTarget : TargetRules
{
    public UE57_Accessible_LabTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("UE57_Accessible_Lab");
    }
}