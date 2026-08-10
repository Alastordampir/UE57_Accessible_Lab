#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccessibilitySubsystem.generated.h"

class UAccessibleObjectComponent;

UENUM(BlueprintType)
enum class EAccessibilityPriority : uint8 { Low, Normal, High, Critical };

UENUM(BlueprintType)
enum class EAccessibilityGameplayMode : uint8 { Any, Exploration, Combat, Dialogue, Menu };

UENUM(BlueprintType, meta = (Bitflags))
enum class EAccessibilityOutputChannel : uint8
{
	None = 0 UMETA(Hidden), Voice = 1 << 0, SpatialAudio = 1 << 1, Haptics = 1 << 2, Visual = 1 << 3
};
ENUM_CLASS_FLAGS(EAccessibilityOutputChannel);

USTRUCT(BlueprintType)
struct UE57_ACCESSIBLE_LAB_API FAccessibilityAnnouncement
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility") FText Message;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (Categories = "Accessibility")) FGameplayTag Category;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility") EAccessibilityPriority Priority = EAccessibilityPriority::Normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility") EAccessibilityGameplayMode GameplayMode = EAccessibilityGameplayMode::Exploration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (Bitmask, BitmaskEnum = "/Script/UE57_Accessible_Lab.EAccessibilityOutputChannel")) int32 AvailableChannels = static_cast<int32>(EAccessibilityOutputChannel::Voice);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility") FName SourceId;
};

USTRUCT(BlueprintType)
struct UE57_ACCESSIBLE_LAB_API FAccessibilityCategoryPreference
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (Categories = "Accessibility")) FGameplayTag Category;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility") EAccessibilityGameplayMode GameplayMode = EAccessibilityGameplayMode::Any;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility") bool bCategoryEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (Bitmask, BitmaskEnum = "/Script/UE57_Accessible_Lab.EAccessibilityOutputChannel")) int32 EnabledChannels = 0x0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility") EAccessibilityPriority MinimumPriority = EAccessibilityPriority::Low;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAccessibilityAnnouncementDynamicDelegate, const FAccessibilityAnnouncement&, Announcement);
DECLARE_MULTICAST_DELEGATE_OneParam(FAccessibilityAnnouncementNativeDelegate, const FAccessibilityAnnouncement&);

/** Routes semantic gameplay announcements after applying runtime user preferences. */
UCLASS()
class UE57_ACCESSIBLE_LAB_API UAccessibilitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable, Category = "Accessibility") FAccessibilityAnnouncementDynamicDelegate OnAnnouncement;
	FAccessibilityAnnouncementNativeDelegate& OnAnnouncementNative() { return NativeAnnouncement; }
	UFUNCTION(BlueprintCallable, Category = "Accessibility") bool SubmitAnnouncement(const FAccessibilityAnnouncement& Announcement);
	UFUNCTION(BlueprintPure, Category = "Accessibility") bool WouldPassFilters(const FAccessibilityAnnouncement& Announcement, FAccessibilityAnnouncement& FilteredAnnouncement) const;
	UFUNCTION(BlueprintCallable, Category = "Accessibility") bool SetCategoryPreference(const FAccessibilityCategoryPreference& Preference);
	UFUNCTION(BlueprintCallable, Category = "Accessibility") bool RemoveCategoryPreference(FGameplayTag Category, EAccessibilityGameplayMode GameplayMode);
	UFUNCTION(BlueprintCallable, Category = "Accessibility") void ClearRuntimePreferences();
	UFUNCTION(BlueprintPure, Category = "Accessibility") bool GetEffectivePreference(FGameplayTag Category, EAccessibilityGameplayMode GameplayMode, FAccessibilityCategoryPreference& Preference) const;
	UFUNCTION(BlueprintCallable, Category = "Accessibility") bool SubmitAccessibleObject(const UAccessibleObjectComponent* Component, FGameplayTag Category, EAccessibilityPriority Priority, EAccessibilityGameplayMode GameplayMode, int32 AvailableChannels, FName SourceId);
private:
	static bool IsConcreteMode(EAccessibilityGameplayMode GameplayMode);
	static int32 ValidChannelMask();
	UPROPERTY(Transient) TArray<FAccessibilityCategoryPreference> RuntimePreferences;
	FAccessibilityAnnouncementNativeDelegate NativeAnnouncement;
};
