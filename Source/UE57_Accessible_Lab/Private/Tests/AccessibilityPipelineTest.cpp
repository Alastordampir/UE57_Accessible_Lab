#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "AccessibilitySubsystem.h"
#include "AccessibleObjectComponent.h"

namespace AccessibilityPipelineTests
{
constexpr int32 Voice = static_cast<int32>(EAccessibilityOutputChannel::Voice);
constexpr int32 Spatial = static_cast<int32>(EAccessibilityOutputChannel::SpatialAudio);
constexpr int32 Haptics = static_cast<int32>(EAccessibilityOutputChannel::Haptics);
constexpr int32 Visual = static_cast<int32>(EAccessibilityOutputChannel::Visual);

FGameplayTag Tag(const TCHAR* Name) { return FGameplayTag::RequestGameplayTag(FName(Name)); }
UAccessibilitySubsystem* Subsystem() { return NewObject<UAccessibilitySubsystem>(); }
FAccessibilityAnnouncement Event(const TCHAR* Category = TEXT("Accessibility.Interaction.Available"))
{
	FAccessibilityAnnouncement Value;
	Value.Message = FText::FromString(TEXT("Porta disponibile"));
	Value.Category = Tag(Category);
	Value.Priority = EAccessibilityPriority::High;
	Value.GameplayMode = EAccessibilityGameplayMode::Exploration;
	Value.AvailableChannels = Voice | Spatial | Visual;
	Value.SourceId = TEXT("Door_01");
	return Value;
}
FAccessibilityCategoryPreference Preference(const TCHAR* Category, EAccessibilityGameplayMode Mode = EAccessibilityGameplayMode::Any)
{
	FAccessibilityCategoryPreference Value;
	Value.Category = Tag(Category); Value.GameplayMode = Mode; Value.EnabledChannels = Voice | Spatial | Haptics | Visual;
	return Value;
}
}

using namespace AccessibilityPipelineTests;
constexpr uint32 Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineTransmissionTest, "UE57AccessibleLab.Accessibility.Pipeline.Transmission", Flags)
bool FPipelineTransmissionTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); int32 Count = 0; FAccessibilityAnnouncement Received;
	Subject->OnAnnouncementNative().AddLambda([&](const FAccessibilityAnnouncement& Value) { ++Count; Received = Value; });
	const FAccessibilityAnnouncement Sent = Event(); TestTrue(TEXT("Valid announcement is accepted"), Subject->SubmitAnnouncement(Sent));
	TestEqual(TEXT("Message is unchanged"), Received.Message.ToString(), Sent.Message.ToString()); TestEqual(TEXT("Category is unchanged"), Received.Category, Sent.Category);
	TestEqual(TEXT("Priority is unchanged"), Received.Priority, Sent.Priority); TestEqual(TEXT("Mode is unchanged"), Received.GameplayMode, Sent.GameplayMode);
	TestEqual(TEXT("Available channels are unchanged"), Received.AvailableChannels, Sent.AvailableChannels);
	TestEqual(TEXT("SourceId is unchanged"), Received.SourceId, Sent.SourceId); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineSingleEmissionTest, "UE57AccessibleLab.Accessibility.Pipeline.SingleEmission", Flags)
bool FPipelineSingleEmissionTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); int32 Count = 0; Subject->OnAnnouncementNative().AddLambda([&](const FAccessibilityAnnouncement&) { ++Count; });
	Subject->SubmitAnnouncement(Event()); TestEqual(TEXT("One request produces one native emission"), Count, 1); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineParentInheritanceTest, "UE57AccessibleLab.Accessibility.Pipeline.ParentInheritance", Flags)
bool FPipelineParentInheritanceTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); auto Rule = Preference(TEXT("Accessibility.Interaction")); Rule.bCategoryEnabled = false; Subject->SetCategoryPreference(Rule);
	FAccessibilityAnnouncement Filtered; TestFalse(TEXT("Child category inherits disabled parent"), Subject->WouldPassFilters(Event(), Filtered)); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineChildOverrideTest, "UE57AccessibleLab.Accessibility.Pipeline.ChildOverride", Flags)
bool FPipelineChildOverrideTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); auto Parent = Preference(TEXT("Accessibility.Interaction")); Parent.bCategoryEnabled = false; Subject->SetCategoryPreference(Parent);
	auto Child = Preference(TEXT("Accessibility.Interaction.Available")); Subject->SetCategoryPreference(Child); FAccessibilityAnnouncement Filtered;
	TestTrue(TEXT("More specific child preference overrides parent"), Subject->WouldPassFilters(Event(), Filtered)); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineModeOverrideTest, "UE57AccessibleLab.Accessibility.Pipeline.ModeOverride", Flags)
bool FPipelineModeOverrideTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); auto General = Preference(TEXT("Accessibility.Interaction")); General.bCategoryEnabled = false; Subject->SetCategoryPreference(General);
	auto Exploration = Preference(TEXT("Accessibility.Interaction"), EAccessibilityGameplayMode::Exploration); Subject->SetCategoryPreference(Exploration); FAccessibilityAnnouncement Filtered;
	TestTrue(TEXT("Concrete mode preference overrides Any"), Subject->WouldPassFilters(Event(), Filtered)); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineChannelFilteringTest, "UE57AccessibleLab.Accessibility.Pipeline.ChannelFiltering", Flags)
bool FPipelineChannelFilteringTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); int32 Count = 0; Subject->OnAnnouncementNative().AddLambda([&](const FAccessibilityAnnouncement&) { ++Count; });
	auto Rule = Preference(TEXT("Accessibility")); Rule.EnabledChannels = Voice | Haptics; Subject->SetCategoryPreference(Rule);
	FAccessibilityAnnouncement Filtered; TestTrue(TEXT("One available enabled channel permits event"), Subject->WouldPassFilters(Event(), Filtered));
	TestEqual(TEXT("Filter evaluation returns the announcement message"), Filtered.Message.ToString(), Event().Message.ToString());
	TestEqual(TEXT("Final channels are the intersection"), Filtered.AvailableChannels, Voice);
	TestEqual(TEXT("Filter evaluation does not emit an announcement"), Count, 0); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineMinimumPriorityTest, "UE57AccessibleLab.Accessibility.Pipeline.MinimumPriority", Flags)
bool FPipelineMinimumPriorityTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); auto Rule = Preference(TEXT("Accessibility")); Rule.MinimumPriority = EAccessibilityPriority::Critical; Subject->SetCategoryPreference(Rule);
	FAccessibilityAnnouncement Filtered; TestFalse(TEXT("Announcement below minimum priority is rejected"), Subject->WouldPassFilters(Event(), Filtered)); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineAccessibleObjectTest, "UE57AccessibleLab.Accessibility.Pipeline.AccessibleObject", Flags)
bool FPipelineAccessibleObjectTest::RunTest(const FString& Parameters)
{
	UAccessibilitySubsystem* Subject = Subsystem(); UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();
	Component->AccessibleName = FText::FromString(TEXT("Leva")); int32 Count = 0; FAccessibilityAnnouncement Received;
	Subject->OnAnnouncementNative().AddLambda([&](const FAccessibilityAnnouncement& Value) { ++Count; Received = Value; });
	TestTrue(TEXT("Enabled accessible component produces an announcement"), Subject->SubmitAccessibleObject(Component, Tag(TEXT("Accessibility.Interaction.Available")), EAccessibilityPriority::Normal, EAccessibilityGameplayMode::Exploration, Voice, TEXT("Lever_01")));
	TestEqual(TEXT("Component announcement text is forwarded"), Received.Message.ToString(), FString(TEXT("Leva"))); TestEqual(TEXT("Component produces one emission"), Count, 1); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineInvalidInputsTest, "UE57AccessibleLab.Accessibility.Pipeline.InvalidInputs", Flags)
bool FPipelineInvalidInputsTest::RunTest(const FString& Parameters)
{
	const EAccessibilityGameplayMode InvalidMode = static_cast<EAccessibilityGameplayMode>(255);
	UAccessibilitySubsystem* Subject = Subsystem(); FAccessibilityAnnouncement Filtered; auto Value = Event(); Value.Message = FText::GetEmpty(); TestFalse(TEXT("Empty message is rejected"), Subject->WouldPassFilters(Value, Filtered));
	Value = Event(); Value.Category = FGameplayTag(); TestFalse(TEXT("Invalid category is rejected"), Subject->WouldPassFilters(Value, Filtered));
	Value = Event(); Value.AvailableChannels = 0; TestFalse(TEXT("No channels is rejected"), Subject->WouldPassFilters(Value, Filtered));
	Value = Event(); Value.GameplayMode = EAccessibilityGameplayMode::Any; TestFalse(TEXT("Any gameplay mode is rejected for announcements"), Subject->WouldPassFilters(Value, Filtered));
	Value = Event(); Value.GameplayMode = InvalidMode; TestFalse(TEXT("Out-of-range gameplay mode is rejected for announcements"), Subject->WouldPassFilters(Value, Filtered));
	auto ExistingPreference = Preference(TEXT("Accessibility.Interaction")); ExistingPreference.bCategoryEnabled = false; TestTrue(TEXT("Any gameplay mode remains valid for preferences"), Subject->SetCategoryPreference(ExistingPreference));
	auto InvalidPreference = ExistingPreference; InvalidPreference.GameplayMode = InvalidMode; InvalidPreference.bCategoryEnabled = true; TestFalse(TEXT("Out-of-range gameplay mode is rejected for preferences"), Subject->SetCategoryPreference(InvalidPreference));
	FAccessibilityCategoryPreference EffectivePreference; TestTrue(TEXT("Existing preference remains available after invalid preference"), Subject->GetEffectivePreference(Tag(TEXT("Accessibility.Interaction.Available")), EAccessibilityGameplayMode::Exploration, EffectivePreference));
	TestFalse(TEXT("Invalid preference does not alter runtime preference state"), EffectivePreference.bCategoryEnabled);
	TestFalse(TEXT("Null component is rejected"), Subject->SubmitAccessibleObject(nullptr, Tag(TEXT("Accessibility.Interaction")), EAccessibilityPriority::Normal, EAccessibilityGameplayMode::Exploration, Voice, NAME_None));
	UAccessibleObjectComponent* Disabled = NewObject<UAccessibleObjectComponent>(); Disabled->AccessibleName = FText::FromString(TEXT("Disabled")); Disabled->bAccessibilityEnabled = false;
	TestFalse(TEXT("Disabled component is rejected"), Subject->SubmitAccessibleObject(Disabled, Tag(TEXT("Accessibility.Interaction")), EAccessibilityPriority::Normal, EAccessibilityGameplayMode::Exploration, Voice, NAME_None)); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPipelineRejectedNoEmissionTest, "UE57AccessibleLab.Accessibility.Pipeline.RejectedNoEmission", Flags)
bool FPipelineRejectedNoEmissionTest::RunTest(const FString& Parameters)
{
	const EAccessibilityGameplayMode InvalidMode = static_cast<EAccessibilityGameplayMode>(255);
	UAccessibilitySubsystem* Subject = Subsystem(); int32 Count = 0; Subject->OnAnnouncementNative().AddLambda([&](const FAccessibilityAnnouncement&) { ++Count; });
	auto Value = Event(); Value.AvailableChannels = 0; TestFalse(TEXT("Rejected request reports failure"), Subject->SubmitAnnouncement(Value));
	Value = Event(); Value.GameplayMode = EAccessibilityGameplayMode::Any; TestFalse(TEXT("Announcement with Any gameplay mode is not submitted"), Subject->SubmitAnnouncement(Value));
	Value = Event(); Value.GameplayMode = InvalidMode; TestFalse(TEXT("Announcement with out-of-range gameplay mode is not submitted"), Subject->SubmitAnnouncement(Value));
	TestEqual(TEXT("Rejected requests emit nothing"), Count, 0); return true;
}

#endif
