#include "AccessibilitySubsystem.h"
#include "AccessibleObjectComponent.h"

int32 UAccessibilitySubsystem::ValidChannelMask()
{
	return static_cast<int32>(EAccessibilityOutputChannel::Voice) | static_cast<int32>(EAccessibilityOutputChannel::SpatialAudio)
		| static_cast<int32>(EAccessibilityOutputChannel::Haptics) | static_cast<int32>(EAccessibilityOutputChannel::Visual);
}

bool UAccessibilitySubsystem::IsConcreteMode(const EAccessibilityGameplayMode GameplayMode)
{
	return GameplayMode != EAccessibilityGameplayMode::Any;
}

bool UAccessibilitySubsystem::GetEffectivePreference(const FGameplayTag Category, const EAccessibilityGameplayMode GameplayMode,
	FAccessibilityCategoryPreference& Preference) const
{
	if (!Category.IsValid() || !IsConcreteMode(GameplayMode)) return false;
	const FAccessibilityCategoryPreference* Best = nullptr;
	int32 BestDepth = -1;
	bool bBestIsConcrete = false;
	for (const FAccessibilityCategoryPreference& Candidate : RuntimePreferences)
	{
		if (!Candidate.Category.IsValid() || !Category.MatchesTag(Candidate.Category)
			|| (Candidate.GameplayMode != EAccessibilityGameplayMode::Any && Candidate.GameplayMode != GameplayMode)) continue;
		TArray<FString> Segments;
		Candidate.Category.ToString().ParseIntoArray(Segments, TEXT("."), true);
		const int32 Depth = Segments.Num();
		const bool bIsConcrete = Candidate.GameplayMode == GameplayMode;
		if (Depth > BestDepth || (Depth == BestDepth && bIsConcrete && !bBestIsConcrete))
		{
			Best = &Candidate; BestDepth = Depth; bBestIsConcrete = bIsConcrete;
		}
	}
	if (Best == nullptr) return false;
	Preference = *Best;
	return true;
}

bool UAccessibilitySubsystem::WouldPassFilters(const FAccessibilityAnnouncement& Announcement, FAccessibilityAnnouncement& FilteredAnnouncement) const
{
	const int32 NormalizedChannels = Announcement.AvailableChannels & ValidChannelMask();
	if (Announcement.Message.IsEmptyOrWhitespace() || !Announcement.Category.IsValid() || !IsConcreteMode(Announcement.GameplayMode) || NormalizedChannels == 0) return false;
	FilteredAnnouncement = Announcement;
	FilteredAnnouncement.AvailableChannels = NormalizedChannels;
	FAccessibilityCategoryPreference Preference;
	if (GetEffectivePreference(Announcement.Category, Announcement.GameplayMode, Preference))
	{
		if (!Preference.bCategoryEnabled || Announcement.Priority < Preference.MinimumPriority) return false;
		FilteredAnnouncement.AvailableChannels &= Preference.EnabledChannels & ValidChannelMask();
	}
	return FilteredAnnouncement.AvailableChannels != 0;
}

bool UAccessibilitySubsystem::SubmitAnnouncement(const FAccessibilityAnnouncement& Announcement)
{
	FAccessibilityAnnouncement Filtered;
	if (!WouldPassFilters(Announcement, Filtered)) return false;
	OnAnnouncement.Broadcast(Filtered);
	NativeAnnouncement.Broadcast(Filtered);
	return true;
}

bool UAccessibilitySubsystem::SetCategoryPreference(const FAccessibilityCategoryPreference& Preference)
{
	if (!Preference.Category.IsValid()) return false;
	for (FAccessibilityCategoryPreference& Existing : RuntimePreferences)
	{
		if (Existing.Category == Preference.Category && Existing.GameplayMode == Preference.GameplayMode)
		{
			Existing = Preference; Existing.EnabledChannels &= ValidChannelMask(); return true;
		}
	}
	FAccessibilityCategoryPreference Normalized = Preference;
	Normalized.EnabledChannels &= ValidChannelMask();
	RuntimePreferences.Add(Normalized);
	return true;
}

bool UAccessibilitySubsystem::RemoveCategoryPreference(const FGameplayTag Category, const EAccessibilityGameplayMode GameplayMode)
{
	return RuntimePreferences.RemoveAll([&](const FAccessibilityCategoryPreference& Preference)
	{
		return Preference.Category == Category && Preference.GameplayMode == GameplayMode;
	}) > 0;
}

void UAccessibilitySubsystem::ClearRuntimePreferences() { RuntimePreferences.Reset(); }

bool UAccessibilitySubsystem::SubmitAccessibleObject(const UAccessibleObjectComponent* Component, const FGameplayTag Category,
	const EAccessibilityPriority Priority, const EAccessibilityGameplayMode GameplayMode, const int32 AvailableChannels, const FName SourceId)
{
	if (Component == nullptr || !Component->IsAvailableForAccessibility()) return false;
	FAccessibilityAnnouncement Announcement;
	Announcement.Message = Component->GetAccessibleAnnouncement(); Announcement.Category = Category; Announcement.Priority = Priority;
	Announcement.GameplayMode = GameplayMode; Announcement.AvailableChannels = AvailableChannels; Announcement.SourceId = SourceId;
	return SubmitAnnouncement(Announcement);
}
