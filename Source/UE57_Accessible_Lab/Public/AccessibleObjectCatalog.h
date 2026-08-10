#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AccessibleObjectCatalog.generated.h"

/** Semantic role used to identify an accessible object. */
UENUM(BlueprintType)
enum class EAccessibleObjectCategory : uint8
{
	Unknown,
	Objective,
	Interactable,
	Enemy,
	Ally,
	Hazard,
	Landmark
};

/** Static, authorable accessibility metadata for one catalogued object. */
USTRUCT(BlueprintType)
struct UE57_ACCESSIBLE_LAB_API FAccessibleObjectCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	FText AccessibleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (MultiLine = "true"))
	FText AccessibleDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	EAccessibleObjectCategory Category = EAccessibleObjectCategory::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))
	int32 NavigationPriority = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bAccessibilityEnabled = true;

	/** Returns true only when every catalog constraint is satisfied. */
	bool IsValid() const;
};
