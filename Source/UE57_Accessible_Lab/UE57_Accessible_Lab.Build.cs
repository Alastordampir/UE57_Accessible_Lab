using UnrealBuildTool;
public class UE57_Accessible_Lab : ModuleRules
{
    public UE57_Accessible_Lab(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayTags",
                "InputCore"
            }
        );
    }
}