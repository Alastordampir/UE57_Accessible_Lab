#include "AccessibleObjectComponent.h"

#define LOCTEXT_NAMESPACE "AccessibleObjectComponent"

UAccessibleObjectComponent::UAccessibleObjectComponent()
	: Category(EAccessibleObjectCategory::Unknown)
	, NavigationPriority(50)
	, bAccessibilityEnabled(true)
{
	PrimaryComponentTick.bCanEverTick = false;
}

FText UAccessibleObjectComponent::GetAccessibleAnnouncement() const
{
	if (AccessibleName.IsEmpty())
	{
		return AccessibleDescription;
	}

	if (AccessibleDescription.IsEmpty())
	{
		return AccessibleName;
	}

	return FText::Format(
		LOCTEXT("AnnouncementFormat", "{0}. {1}"),
		AccessibleName,
		AccessibleDescription);
}

bool UAccessibleObjectComponent::IsAvailableForAccessibility() const
{
	return bAccessibilityEnabled;
}

#undef LOCTEXT_NAMESPACE
