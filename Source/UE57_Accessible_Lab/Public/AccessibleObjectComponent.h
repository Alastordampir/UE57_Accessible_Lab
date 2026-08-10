#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AccessibleObjectComponent.generated.h"

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

/** Supplies player-facing accessibility metadata for an Actor. */
UCLASS(ClassGroup = (Accessibility), meta = (BlueprintSpawnableComponent))
class UE57_ACCESSIBLE_LAB_API UAccessibleObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAccessibleObjectComponent();

	/** Short player-facing name announced for this object. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	FText AccessibleName;

	/** Additional player-facing context announced after the name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (MultiLine = "true"))
	FText AccessibleDescription;

	/** Semantic role of this object in the accessible experience. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	EAccessibleObjectCategory Category;

	/** Relative importance when accessible objects are presented to the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))
	int32 NavigationPriority;

	/** Whether this object's accessibility metadata is currently available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bAccessibilityEnabled;

	/** Builds the localized announcement text from the available name and description. */
	UFUNCTION(BlueprintPure, Category = "Accessibility")
	FText GetAccessibleAnnouncement() const;

	/** Returns whether this object is enabled for accessibility systems. */
	UFUNCTION(BlueprintPure, Category = "Accessibility")
	bool IsAvailableForAccessibility() const;
};
