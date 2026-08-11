# Architettura di accessibilità

## Flusso semantico

I sistemi di gameplay descrivono **che cosa è accaduto**, senza scegliere una tecnologia: gameplay → `FAccessibilityAnnouncement` → `UAccessibilitySubsystem` → preferenze runtime → delegate → adattatori futuri. Il componente `UAccessibleObjectComponent` resta responsabile soltanto dei metadati localizzati di un attore (nome, descrizione, ruolo, priorità di navigazione e disponibilità); non instrada output. Il subsystem, con durata pari alla game instance, valida, filtra e distribuisce eventi e non conserva riferimenti agli attori.

Il catalogo degli oggetti aggiunge a monte una fonte di authoring testuale: `CSV → Data Table → FDataTableRowHandle → UAccessibleObjectComponent → UAccessibilitySubsystem → delegate`. La colonna CSV `Name` è il RowName stabile; non viene copiata in un campo identificativo della riga. I cinque valori statici vengono applicati atomicamente soltanto dopo la validazione completa. Tabella o RowName nulli, tabella con struttura di riga incompatibile, riga assente, categoria non concreta, priorità fuori da 0–100 e nome e descrizione entrambi privi di testo falliscono senza mutare il componente e senza log del motore. Il CSV resta una fonte revisionabile e un dato dei test, non un file caricato direttamente a runtime; non sono stati aggiunti asset binari.

Un annuncio contiene messaggio localizzato, Gameplay Tag di categoria, priorità (`Low`–`Critical`), modalità concreta (`Exploration`, `Combat`, `Dialogue` o `Menu`), maschera dei canali disponibili (`Voice`, `SpatialAudio`, `Haptics`, `Visual`) e `SourceId` leggero. `Any` è riservato alle preferenze. Le categorie gerarchiche classificano eventi, non identificano oggetti, nemici, quest o dialoghi.

## Risoluzione delle preferenze

Le preferenze esistono solo in memoria. Per la categoria dell'evento si considerano le regole della categoria stessa e degli antenati; vince la categoria con più segmenti. A pari categoria, la modalità concreta vince su `Any`. Una regola può disabilitare la categoria, imporre la priorità minima e abilitare canali indipendenti. Senza regola l'evento valido passa invariato. La maschera finale è l'intersezione fra canali disponibili normalizzati e canali abilitati; una maschera vuota o una priorità insufficiente impedisce l'unica emissione.

## Separazione e stato di gameplay

Gameplay, presentazione e accessibilità restano separati. L'esplorazione sarà in prima persona e il combattimento tattico passerà alla terza persona, ma **la telecamera non determina la modalità**. Un futuro sistema autorevole dichiarerà `Exploration` o `Combat`; telecamera, input, accessibilità, audio e aptica reagiranno allo stesso stato.

Il delegate Blueprint e quello C++ sono punti di estensione per adattatori futuri di voce/screen reader, audio spaziale, aptica e visuale. Questa prima implementazione non include NVDA, screen reader, MetaSound, DualSense, focus, sonar o navigazione sonora.

## Limiti e prossimi passi

Non sono inclusi persistenza, `UGameUserSettings`, menu o profili predefiniti. Il catalogo corrente riguarda esclusivamente gli oggetti accessibili e non introduce quest manager, sistemi di dialogo o combattimento. Cataloghi separati per altri domini, importazione gestita dagli strumenti di progetto, localizzazione completa e adattatori reali restano possibili passi futuri e richiederanno decisioni dedicate. Il CSV va mantenuto UTF-8 con intestazioni stabili e può essere modificato con Excel o un editor testuale.
