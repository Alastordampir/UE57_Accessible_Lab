#if WITH_DEV_AUTOMATION_TESTS

#include "AccessibleObjectComponent.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAccessibleObjectDefaultsTest,
	"UE57AccessibleLab.Accessibility.AccessibleObject.Defaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibleObjectDefaultsTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();

	TestNotNull(TEXT("Il componente viene creato in memoria"), Component);
	if (Component == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("La categoria predefinita e Unknown"), Component->Category, EAccessibleObjectCategory::Unknown);
	TestEqual(TEXT("La priorita di navigazione predefinita e 50"), Component->NavigationPriority, 50);
	TestTrue(TEXT("L'accessibilita e abilitata per impostazione predefinita"), Component->bAccessibilityEnabled);
	TestTrue(TEXT("Il componente e inizialmente disponibile"), Component->IsAvailableForAccessibility());
	TestEqual(TEXT("Il nome accessibile e inizialmente vuoto"), Component->AccessibleName.ToString(), FString());
	TestEqual(TEXT("La descrizione accessibile e inizialmente vuota"), Component->AccessibleDescription.ToString(), FString());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAccessibleObjectAnnouncementTest,
	"UE57AccessibleLab.Accessibility.AccessibleObject.Announcement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibleObjectAnnouncementTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();

	TestNotNull(TEXT("Il componente viene creato in memoria"), Component);
	if (Component == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("Nome e descrizione vuoti producono un annuncio vuoto"), Component->GetAccessibleAnnouncement().ToString(), FString());

	Component->AccessibleName = FText::FromString(TEXT("Nome"));
	TestEqual(TEXT("Il solo nome viene restituito senza modifiche"), Component->GetAccessibleAnnouncement().ToString(), FString(TEXT("Nome")));

	Component->AccessibleName = FText::GetEmpty();
	Component->AccessibleDescription = FText::FromString(TEXT("Descrizione"));
	TestEqual(TEXT("La sola descrizione viene restituita senza modifiche"), Component->GetAccessibleAnnouncement().ToString(), FString(TEXT("Descrizione")));

	Component->AccessibleName = FText::FromString(TEXT("Nome"));
	TestEqual(TEXT("Nome e descrizione vengono separati da punto e spazio"), Component->GetAccessibleAnnouncement().ToString(), FString(TEXT("Nome. Descrizione")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAccessibleObjectAvailabilityTest,
	"UE57AccessibleLab.Accessibility.AccessibleObject.Availability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibleObjectAvailabilityTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();

	TestNotNull(TEXT("Il componente viene creato in memoria"), Component);
	if (Component == nullptr)
	{
		return false;
	}

	Component->bAccessibilityEnabled = true;
	TestTrue(TEXT("Il componente abilitato e disponibile"), Component->IsAvailableForAccessibility());

	Component->bAccessibilityEnabled = false;
	TestFalse(TEXT("Il componente disabilitato non e disponibile"), Component->IsAvailableForAccessibility());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
