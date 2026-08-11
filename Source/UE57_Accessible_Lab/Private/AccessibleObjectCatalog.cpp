#include "AccessibleObjectCatalog.h"

bool FAccessibleObjectCatalogRow::IsValid() const
{
	if ((AccessibleName.IsEmptyOrWhitespace() && AccessibleDescription.IsEmptyOrWhitespace())
		|| NavigationPriority < 0 || NavigationPriority > 100)
	{
		return false;
	}

	switch (Category)
	{
	case EAccessibleObjectCategory::Objective:
	case EAccessibleObjectCategory::Interactable:
	case EAccessibleObjectCategory::Enemy:
	case EAccessibleObjectCategory::Ally:
	case EAccessibleObjectCategory::Hazard:
	case EAccessibleObjectCategory::Landmark:
		return true;
	case EAccessibleObjectCategory::Unknown:
	default:
		return false;
	}
}
