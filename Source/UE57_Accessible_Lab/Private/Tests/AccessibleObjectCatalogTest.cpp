#if WITH_DEV_AUTOMATION_TESTS

#include "AccessibleObjectCatalog.h"
#include "AccessibleObjectComponent.h"
#include "AccessibilitySubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

namespace AccessibleObjectCatalogTests
{
constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;
constexpr int32 Voice = static_cast<int32>(EAccessibilityOutputChannel::Voice);

UDataTable* ImportCatalog(FAutomationTestBase& Test)
{
	FString Csv;
	const FString CsvPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Data/Catalogs/AccessibleObjects.csv"));
	if (!Test.TestTrue(TEXT("Il CSV del catalogo viene letto dal repository"), FFileHelper::LoadFileToString(Csv, *CsvPath)))
	{
		return nullptr;
	}

	UDataTable* Table = NewObject<UDataTable>();
	Table->RowStruct = FAccessibleObjectCatalogRow::StaticStruct();
	const TArray<FString> ImportProblems = Table->CreateTableFromCSVString(Csv);
	Test.TestEqual(TEXT("Il CSV viene importato senza problemi"), ImportProblems.Num(), 0);
	return ImportProblems.IsEmpty() ? Table : nullptr;
}

FAccessibleObjectCatalogRow ValidRow()
{
	FAccessibleObjectCatalogRow Row;
	Row.AccessibleName = FText::FromString(TEXT("Oggetto iniziale"));
	Row.AccessibleDescription = FText::FromString(TEXT("Descrizione iniziale"));
	Row.Category = EAccessibleObjectCategory::Landmark;
	Row.NavigationPriority = 42;
	Row.bAccessibilityEnabled = false;
	return Row;
}

void SetRecognizableState(UAccessibleObjectComponent& Component)
{
	const FAccessibleObjectCatalogRow Row = ValidRow();
	Component.ApplyCatalogRow(Row);
}

void TestRecognizableState(FAutomationTestBase& Test, const UAccessibleObjectComponent& Component, const TCHAR* Context)
{
	Test.TestEqual(FString::Printf(TEXT("%s: nome invariato"), Context), Component.AccessibleName.ToString(), FString(TEXT("Oggetto iniziale")));
	Test.TestEqual(FString::Printf(TEXT("%s: descrizione invariata"), Context), Component.AccessibleDescription.ToString(), FString(TEXT("Descrizione iniziale")));
	Test.TestEqual(FString::Printf(TEXT("%s: categoria invariata"), Context), Component.Category, EAccessibleObjectCategory::Landmark);
	Test.TestEqual(FString::Printf(TEXT("%s: priorita invariata"), Context), Component.NavigationPriority, 42);
	Test.TestFalse(FString::Printf(TEXT("%s: abilitazione invariata"), Context), Component.bAccessibilityEnabled);
}
}

using namespace AccessibleObjectCatalogTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogCsvImportTest,
	"UE57AccessibleLab.Accessibility.Catalog.CsvImport", Flags)
bool FAccessibleObjectCatalogCsvImportTest::RunTest(const FString& Parameters)
{
	UDataTable* Table = ImportCatalog(*this);
	if (Table == nullptr) return false;

	const TArray<FName> RowNames = Table->GetRowNames();
	TestEqual(TEXT("Il catalogo contiene esattamente tre righe"), RowNames.Num(), 3);
	TestTrue(TEXT("PrototypeSword e presente"), RowNames.Contains(FName(TEXT("PrototypeSword"))));
	TestTrue(TEXT("TrainingEnemy e presente"), RowNames.Contains(FName(TEXT("TrainingEnemy"))));
	TestTrue(TEXT("QuestDestination e presente"), RowNames.Contains(FName(TEXT("QuestDestination"))));

	const FAccessibleObjectCatalogRow* Sword = Table->FindRow<FAccessibleObjectCatalogRow>(FName(TEXT("PrototypeSword")), TEXT("CatalogCsvImport"), false);
	const FAccessibleObjectCatalogRow* Enemy = Table->FindRow<FAccessibleObjectCatalogRow>(FName(TEXT("TrainingEnemy")), TEXT("CatalogCsvImport"), false);
	const FAccessibleObjectCatalogRow* Destination = Table->FindRow<FAccessibleObjectCatalogRow>(FName(TEXT("QuestDestination")), TEXT("CatalogCsvImport"), false);
	TestNotNull(TEXT("La riga della spada viene importata"), Sword);
	TestNotNull(TEXT("La riga del nemico viene importata"), Enemy);
	TestNotNull(TEXT("La riga dell'obiettivo viene importata"), Destination);
	if (Sword == nullptr || Enemy == nullptr || Destination == nullptr) return false;

	TestEqual(TEXT("La spada ha il nome atteso"), Sword->AccessibleName.ToString(), FString(TEXT("Spada del prototipo")));
	TestEqual(TEXT("La spada e interagibile"), Sword->Category, EAccessibleObjectCategory::Interactable);
	TestEqual(TEXT("Il nemico ha la categoria attesa"), Enemy->Category, EAccessibleObjectCategory::Enemy);
	TestEqual(TEXT("Il nemico ha priorita 80"), Enemy->NavigationPriority, 80);
	TestEqual(TEXT("La destinazione ha la categoria attesa"), Destination->Category, EAccessibleObjectCategory::Objective);
	TestTrue(TEXT("La destinazione e abilitata"), Destination->bAccessibilityEnabled);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogApplyRowTest,
	"UE57AccessibleLab.Accessibility.Catalog.ApplyRow", Flags)
bool FAccessibleObjectCatalogApplyRowTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();
	FAccessibleObjectCatalogRow Row;
	Row.AccessibleName = FText::FromString(TEXT("Ponte"));
	Row.AccessibleDescription = FText::FromString(TEXT("Attraversamento sicuro"));
	Row.Category = EAccessibleObjectCategory::Landmark;
	Row.NavigationPriority = 65;
	Row.bAccessibilityEnabled = false;

	TestTrue(TEXT("Una riga valida viene applicata"), Component->ApplyCatalogRow(Row));
	TestEqual(TEXT("Il nome viene sostituito"), Component->AccessibleName.ToString(), FString(TEXT("Ponte")));
	TestEqual(TEXT("La descrizione viene sostituita"), Component->AccessibleDescription.ToString(), FString(TEXT("Attraversamento sicuro")));
	TestEqual(TEXT("La categoria viene sostituita"), Component->Category, EAccessibleObjectCategory::Landmark);
	TestEqual(TEXT("La priorita viene sostituita"), Component->NavigationPriority, 65);
	TestFalse(TEXT("Una riga disabilitata valida sostituisce l'abilitazione"), Component->bAccessibilityEnabled);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogMissingRowNoMutationTest,
	"UE57AccessibleLab.Accessibility.Catalog.MissingRowNoMutation", Flags)
bool FAccessibleObjectCatalogMissingRowNoMutationTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();
	SetRecognizableState(*Component);
	UDataTable* Table = ImportCatalog(*this);
	if (Table == nullptr) return false;
	FDataTableRowHandle Handle;
	Handle.DataTable = Table;
	Handle.RowName = FName(TEXT("MissingObject"));
	TestFalse(TEXT("Una riga assente viene rifiutata senza warning"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("Riga assente"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogInvalidRowNoMutationTest,
	"UE57AccessibleLab.Accessibility.Catalog.InvalidRowNoMutation", Flags)
bool FAccessibleObjectCatalogInvalidRowNoMutationTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();
	SetRecognizableState(*Component);

	FAccessibleObjectCatalogRow Invalid = ValidRow();
	Invalid.NavigationPriority = 101;
	TestFalse(TEXT("Una priorita sopra il limite viene rifiutata"), Component->ApplyCatalogRow(Invalid));
	TestRecognizableState(*this, *Component, TEXT("Priorita alta"));

	Invalid = ValidRow();
	Invalid.NavigationPriority = -1;
	TestFalse(TEXT("Una priorita sotto il limite viene rifiutata"), Component->ApplyCatalogRow(Invalid));
	TestRecognizableState(*this, *Component, TEXT("Priorita bassa"));

	Invalid = ValidRow();
	Invalid.Category = EAccessibleObjectCategory::Unknown;
	TestFalse(TEXT("Unknown viene rifiutata"), Component->ApplyCatalogRow(Invalid));
	TestRecognizableState(*this, *Component, TEXT("Categoria Unknown"));

	Invalid = ValidRow();
	Invalid.Category = static_cast<EAccessibleObjectCategory>(255);
	TestFalse(TEXT("Una categoria fuori intervallo viene rifiutata"), Component->ApplyCatalogRow(Invalid));
	TestRecognizableState(*this, *Component, TEXT("Categoria fuori intervallo"));

	Invalid = ValidRow();
	Invalid.AccessibleName = FText::FromString(TEXT("   "));
	Invalid.AccessibleDescription = FText::GetEmpty();
	TestFalse(TEXT("Nome e descrizione vuoti vengono rifiutati"), Component->ApplyCatalogRow(Invalid));
	TestRecognizableState(*this, *Component, TEXT("Testo vuoto"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogPipelineIntegrationTest,
	"UE57AccessibleLab.Accessibility.Catalog.PipelineIntegration", Flags)
bool FAccessibleObjectCatalogPipelineIntegrationTest::RunTest(const FString& Parameters)
{
	UDataTable* Table = ImportCatalog(*this);
	if (Table == nullptr) return false;
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();
	FDataTableRowHandle Handle;
	Handle.DataTable = Table;
	Handle.RowName = FName(TEXT("PrototypeSword"));
	TestTrue(TEXT("La riga attraversa il Data Table row handle"), Component->LoadFromCatalog(Handle));

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UAccessibilitySubsystem* Subsystem = NewObject<UAccessibilitySubsystem>(GameInstance);
	int32 EmissionCount = 0;
	FAccessibilityAnnouncement Received;
	Subsystem->OnAnnouncementNative().AddLambda([&](const FAccessibilityAnnouncement& Announcement)
	{
		++EmissionCount;
		Received = Announcement;
	});
	const FGameplayTag EventCategory = FGameplayTag::RequestGameplayTag(FName(TEXT("Accessibility.Interaction.Available")));
	TestTrue(TEXT("Il componente caricato viene inviato alla pipeline"), Subsystem->SubmitAccessibleObject(
		Component, EventCategory, EAccessibilityPriority::High, EAccessibilityGameplayMode::Exploration, Voice, Handle.RowName));
	TestEqual(TEXT("La pipeline emette una sola volta"), EmissionCount, 1);
	TestEqual(TEXT("Il messaggio proviene dalla riga del catalogo"), Received.Message.ToString(), Component->GetAccessibleAnnouncement().ToString());
	TestEqual(TEXT("Il messaggio contiene il nome importato"), Component->AccessibleName.ToString(), FString(TEXT("Spada del prototipo")));
	TestEqual(TEXT("La categoria evento viene conservata"), Received.Category, EventCategory);
	TestEqual(TEXT("Il RowName diventa il SourceId"), Received.SourceId, Handle.RowName);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
