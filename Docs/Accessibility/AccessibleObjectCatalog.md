# Catalogo degli oggetti accessibili

## Ruolo e flusso dei dati

Il catalogo raccoglie i metadati statici e revisionabili che descrivono gli oggetti al livello di accessibilità. La fonte di authoring è `Data/Catalogs/AccessibleObjects.csv`; i test la importano in una Unreal Data Table in memoria con righe `FAccessibleObjectCatalogRow`. Il percorso previsto è:

`CSV → Data Table → FDataTableRowHandle → UAccessibleObjectComponent → UAccessibilitySubsystem → delegate`

`ApplyCatalogRow` e `LoadFromCatalog` applicano insieme nome, descrizione, categoria, priorità di navigazione e abilitazione. Prima dell'assegnazione viene validata l'intera riga: un errore restituisce `false` e lascia invariati tutti e cinque i campi. Il catalogo non carica il CSV grezzo durante il gioco e questa fondazione non richiede un'importazione manuale nell'Editor.

## Formato CSV

L'intestazione deve restare, nello stesso ordine:

```csv
Name,AccessibleName,AccessibleDescription,Category,NavigationPriority,bAccessibilityEnabled
```

- `Name` è il RowName della Data Table e l'identificativo stabile, leggibile e univoco. Non esiste un campo ID duplicato nella struttura.
- `AccessibleName` e `AccessibleDescription` sono testi destinati al giocatore.
- `Category` ammette `Objective`, `Interactable`, `Enemy`, `Ally`, `Hazard` o `Landmark`.
- `NavigationPriority` è un intero fra 0 e 100 inclusi.
- `bAccessibilityEnabled` accetta `true` o `false`; anche una riga disabilitata è valida.

La riga è valida soltanto se la categoria è concreta, la priorità è nell'intervallo e almeno uno fra nome e descrizione contiene caratteri diversi da spazi. `Unknown`, valori enum fuori intervallo e due testi vuoti o composti soltanto da spazi sono rifiutati.

## Authoring accessibile

Il CSV può essere modificato con Excel, un foglio di calcolo capace di esportare CSV oppure un editor testuale. Occorre preservare la codifica UTF-8, la prima colonna `Name`, tutte le intestazioni e i nomi delle categorie. Prima di cambiare un `Name`, considerare che può essere referenziato come RowName stabile. Il file testuale consente diff e review senza aprire asset binari; questa PR non aggiunge `.uasset`.

## Confine tra catalogo e runtime

Il catalogo contiene soltanto i cinque dati statici copiati nel componente. Stato del mondo, disponibilità determinata dal gameplay, preferenze utente, modalità corrente, canali di output e `SourceId` dell'evento restano responsabilità runtime. Il flag statico fornisce il valore iniziale del componente, ma non introduce persistenza o gestione delle quest.

I campi `FText` preparano l'integrazione con gli strumenti Unreal, ma il CSV dimostrativo non definisce ancora la futura pipeline completa di localizzazione. Non sono inclusi loader runtime del CSV, adattatori di output, screen reader, sonar, pathfinding, combattimento o sistemi di dialogo.
