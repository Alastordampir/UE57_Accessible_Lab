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
// Prefixed names: the pipeline test declares its own Flags and Voice, and both files would
// collide at global scope if the module ever compiles with unity builds enabled.
constexpr EAutomationTestFlags CatalogFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;
constexpr int32 CatalogVoice = static_cast<int32>(EAccessibilityOutputChannel::Voice);

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

/** Builds an in-memory catalog Data Table holding a single row, valid or not. */
UDataTable* MakeCatalogTable(FName RowName, const FAccessibleObjectCatalogRow& Row)
{
	UDataTable* Table = NewObject<UDataTable>();
	Table->RowStruct = FAccessibleObjectCatalogRow::StaticStruct();
	Table->AddRow(RowName, Row);
	return Table;
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
	"UE57AccessibleLab.Accessibility.Catalog.CsvImport", CatalogFlags)
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
	// Le celle FText devono arrivare intere, con spazi e apostrofi.
	TestEqual(TEXT("La spada conserva la descrizione completa"), Sword->AccessibleDescription.ToString(), FString(TEXT("Raccogli la spada per provarne l'interazione.")));
	TestEqual(TEXT("La spada e interagibile"), Sword->Category, EAccessibleObjectCategory::Interactable);
	TestEqual(TEXT("La spada ha priorita 70"), Sword->NavigationPriority, 70);
	TestEqual(TEXT("Il nemico ha la categoria attesa"), Enemy->Category, EAccessibleObjectCategory::Enemy);
	TestEqual(TEXT("Il nemico ha priorita 80"), Enemy->NavigationPriority, 80);
	TestEqual(TEXT("La destinazione ha la categoria attesa"), Destination->Category, EAccessibleObjectCategory::Objective);
	TestEqual(TEXT("La destinazione ha priorita 90"), Destination->NavigationPriority, 90);
	TestTrue(TEXT("La destinazione e abilitata"), Destination->bAccessibilityEnabled);

	// Ogni riga del CSV dimostrativo deve superare la validazione.
	TestTrue(TEXT("La riga della spada e valida"), Sword->IsValid());
	TestTrue(TEXT("La riga del nemico e valida"), Enemy->IsValid());
	TestTrue(TEXT("La riga dell'obiettivo e valida"), Destination->IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogApplyRowTest,
	"UE57AccessibleLab.Accessibility.Catalog.ApplyRow", CatalogFlags)
bool FAccessibleObjectCatalogApplyRowTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();

	// Una riga valida ma disabilitata sostituisce comunque i valori predefiniti.
	TestTrue(TEXT("Una riga valida disabilitata viene applicata"), Component->ApplyCatalogRow(ValidRow()));
	TestRecognizableState(*this, *Component, TEXT("Prima applicazione"));

	// Partendo da uno stato non predefinito, tutti e cinque i campi devono cambiare.
	FAccessibleObjectCatalogRow Row;
	Row.AccessibleName = FText::FromString(TEXT("Ponte"));
	Row.AccessibleDescription = FText::FromString(TEXT("Attraversamento sicuro"));
	Row.Category = EAccessibleObjectCategory::Interactable;
	Row.NavigationPriority = 65;
	Row.bAccessibilityEnabled = true;

	TestTrue(TEXT("Una riga valida viene applicata"), Component->ApplyCatalogRow(Row));
	TestEqual(TEXT("Il nome viene sostituito"), Component->AccessibleName.ToString(), FString(TEXT("Ponte")));
	TestEqual(TEXT("La descrizione viene sostituita"), Component->AccessibleDescription.ToString(), FString(TEXT("Attraversamento sicuro")));
	TestEqual(TEXT("La categoria viene sostituita"), Component->Category, EAccessibleObjectCategory::Interactable);
	TestEqual(TEXT("La priorita viene sostituita"), Component->NavigationPriority, 65);
	TestTrue(TEXT("L'abilitazione viene sostituita"), Component->bAccessibilityEnabled);

	// I limiti della priorita sono inclusivi.
	Row.NavigationPriority = 0;
	TestTrue(TEXT("La priorita 0 viene accettata"), Component->ApplyCatalogRow(Row));
	TestEqual(TEXT("La priorita 0 viene scritta"), Component->NavigationPriority, 0);
	Row.NavigationPriority = 100;
	TestTrue(TEXT("La priorita 100 viene accettata"), Component->ApplyCatalogRow(Row));
	TestEqual(TEXT("La priorita 100 viene scritta"), Component->NavigationPriority, 100);

	// Basta uno solo fra nome e descrizione per rendere valida la riga.
	Row.AccessibleName = FText::GetEmpty();
	TestTrue(TEXT("La sola descrizione rende valida la riga"), Component->ApplyCatalogRow(Row));
	Row.AccessibleName = FText::FromString(TEXT("Ponte"));
	Row.AccessibleDescription = FText::FromString(TEXT("   "));
	TestTrue(TEXT("Il solo nome rende valida la riga"), Component->ApplyCatalogRow(Row));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogMissingRowNoMutationTest,
	"UE57AccessibleLab.Accessibility.Catalog.MissingRowNoMutation", CatalogFlags)
bool FAccessibleObjectCatalogMissingRowNoMutationTest::RunTest(const FString& Parameters)
{
	UAccessibleObjectComponent* Component = NewObject<UAccessibleObjectComponent>();
	SetRecognizableState(*Component);
	UDataTable* Table = ImportCatalog(*this);
	if (Table == nullptr) return false;

	// Data Table nulla.
	FDataTableRowHandle Handle;
	Handle.RowName = FName(TEXT("PrototypeSword"));
	TestFalse(TEXT("Una tabella nulla viene rifiutata"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("Tabella nulla"));

	// RowName assente.
	Handle.DataTable = Table;
	Handle.RowName = NAME_None;
	TestFalse(TEXT("Un RowName None viene rifiutato"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("RowName None"));

	// Riga inesistente in una tabella valida.
	Handle.RowName = FName(TEXT("MissingObject"));
	TestFalse(TEXT("Una riga assente viene rifiutata senza warning"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("Riga assente"));

	// Tabella senza RowStruct: FindRow registrerebbe un errore di log, quindi non va raggiunta.
	UDataTable* WithoutRowStruct = NewObject<UDataTable>();
	Handle.DataTable = WithoutRowStruct;
	Handle.RowName = FName(TEXT("PrototypeSword"));
	TestFalse(TEXT("Una tabella senza RowStruct viene rifiutata"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("RowStruct assente"));

	// Tabella con una struttura di riga incompatibile.
	UDataTable* IncompatibleTable = NewObject<UDataTable>();
	IncompatibleTable->RowStruct = FAccessibilityAnnouncement::StaticStruct();
	Handle.DataTable = IncompatibleTable;
	TestFalse(TEXT("Una tabella con RowStruct incompatibile viene rifiutata"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("RowStruct incompatibile"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogInvalidRowNoMutationTest,
	"UE57AccessibleLab.Accessibility.Catalog.InvalidRowNoMutation", CatalogFlags)
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

	Invalid = ValidRow();
	Invalid.AccessibleName = FText::GetEmpty();
	Invalid.AccessibleDescription = FText::GetEmpty();
	TestFalse(TEXT("Due testi vuoti vengono rifiutati"), Component->ApplyCatalogRow(Invalid));
	TestRecognizableState(*this, *Component, TEXT("Testi entrambi vuoti"));

	Invalid = ValidRow();
	Invalid.AccessibleName = FText::FromString(TEXT(" \t "));
	Invalid.AccessibleDescription = FText::FromString(TEXT("   "));
	TestFalse(TEXT("Due testi di soli spazi vengono rifiutati"), Component->ApplyCatalogRow(Invalid));
	TestRecognizableState(*this, *Component, TEXT("Testi di soli spazi"));

	// La stessa riga invalida deve essere rifiutata anche risolta da un FDataTableRowHandle.
	FDataTableRowHandle Handle;
	Handle.DataTable = MakeCatalogTable(FName(TEXT("BrokenObject")), Invalid);
	Handle.RowName = FName(TEXT("BrokenObject"));
	TestFalse(TEXT("Una riga invalida caricata dal Data Table viene rifiutata"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("Riga invalida dal Data Table"));

	Invalid = ValidRow();
	Invalid.Category = EAccessibleObjectCategory::Unknown;
	Handle.DataTable = MakeCatalogTable(FName(TEXT("UnknownObject")), Invalid);
	Handle.RowName = FName(TEXT("UnknownObject"));
	TestFalse(TEXT("Una categoria Unknown viene rifiutata anche dal Data Table"), Component->LoadFromCatalog(Handle));
	TestRecognizableState(*this, *Component, TEXT("Unknown dal Data Table"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibleObjectCatalogPipelineIntegrationTest,
	"UE57AccessibleLab.Accessibility.Catalog.PipelineIntegration", CatalogFlags)
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
	TestTrue(TEXT("Il subsystem ha una game instance valida come Outer"), Subsystem->GetGameInstance() == GameInstance);
	int32 EmissionCount = 0;
	FAccessibilityAnnouncement Received;
	Subsystem->OnAnnouncementNative().AddLambda([&](const FAccessibilityAnnouncement& Announcement)
	{
		++EmissionCount;
		Received = Announcement;
	});
	const FGameplayTag EventCategory = FGameplayTag::RequestGameplayTag(FName(TEXT("Accessibility.Interaction.Available")));
	TestTrue(TEXT("Il componente caricato viene inviato alla pipeline"), Subsystem->SubmitAccessibleObject(
		Component, EventCategory, EAccessibilityPriority::High, EAccessibilityGameplayMode::Exploration, CatalogVoice, Handle.RowName));
	TestEqual(TEXT("La pipeline emette una sola volta"), EmissionCount, 1);
	TestEqual(TEXT("Il messaggio proviene dalla riga del catalogo"), Received.Message.ToString(), Component->GetAccessibleAnnouncement().ToString());
	TestEqual(TEXT("Il messaggio contiene il nome importato"), Component->AccessibleName.ToString(), FString(TEXT("Spada del prototipo")));
	TestEqual(TEXT("La categoria evento viene conservata"), Received.Category, EventCategory);
	TestEqual(TEXT("Il RowName diventa il SourceId"), Received.SourceId, Handle.RowName);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
