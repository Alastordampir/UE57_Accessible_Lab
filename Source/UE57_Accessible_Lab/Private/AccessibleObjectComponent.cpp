#include "AccessibleObjectComponent.h"
#include "AccessibleObjectCatalog.h"

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

bool UAccessibleObjectComponent::ApplyCatalogRow(const FAccessibleObjectCatalogRow& Row)
{
	if (!Row.IsValid())
	{
		return false;
	}

	AccessibleName = Row.AccessibleName;
	AccessibleDescription = Row.AccessibleDescription;
	Category = Row.Category;
	NavigationPriority = Row.NavigationPriority;
	bAccessibilityEnabled = Row.bAccessibilityEnabled;
	return true;
}

bool UAccessibleObjectComponent::LoadFromCatalog(const FDataTableRowHandle& RowHandle)
{
	if (RowHandle.DataTable == nullptr || RowHandle.RowName.IsNone())
	{
		return false;
	}

	const FAccessibleObjectCatalogRow* Row = RowHandle.DataTable->FindRow<FAccessibleObjectCatalogRow>(
		RowHandle.RowName, TEXT("AccessibleObjectCatalog"), false);
	return Row != nullptr && ApplyCatalogRow(*Row);
}

#undef LOCTEXT_NAMESPACE
