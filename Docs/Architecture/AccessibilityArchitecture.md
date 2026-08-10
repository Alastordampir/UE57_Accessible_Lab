# Architettura di accessibilità

## Flusso semantico

I sistemi di gameplay descrivono **che cosa è accaduto**, senza scegliere una tecnologia: gameplay → `FAccessibilityAnnouncement` → `UAccessibilitySubsystem` → preferenze runtime → delegate → adattatori futuri. Il componente `UAccessibleObjectComponent` resta responsabile soltanto dei metadati localizzati di un attore (nome, descrizione, ruolo, priorità di navigazione e disponibilità); non instrada output. Il subsystem, con durata pari alla game instance, valida, filtra e distribuisce eventi e non conserva riferimenti agli attori.

Un annuncio contiene messaggio localizzato, Gameplay Tag di categoria, priorità (`Low`–`Critical`), modalità concreta (`Exploration`, `Combat`, `Dialogue` o `Menu`), maschera dei canali disponibili (`Voice`, `SpatialAudio`, `Haptics`, `Visual`) e `SourceId` leggero. `Any` è riservato alle preferenze. Le categorie gerarchiche classificano eventi, non identificano oggetti, nemici, quest o dialoghi.

## Risoluzione delle preferenze

Le preferenze esistono solo in memoria. Per la categoria dell'evento si considerano le regole della categoria stessa e degli antenati; vince la categoria con più segmenti. A pari categoria, la modalità concreta vince su `Any`. Una regola può disabilitare la categoria, imporre la priorità minima e abilitare canali indipendenti. Senza regola l'evento valido passa invariato. La maschera finale è l'intersezione fra canali disponibili normalizzati e canali abilitati; una maschera vuota o una priorità insufficiente impedisce l'unica emissione.

## Separazione e stato di gameplay

Gameplay, presentazione e accessibilità restano separati. L'esplorazione sarà in prima persona e il combattimento tattico passerà alla terza persona, ma **la telecamera non determina la modalità**. Un futuro sistema autorevole dichiarerà `Exploration` o `Combat`; telecamera, input, accessibilità, audio e aptica reagiranno allo stesso stato.

Il delegate Blueprint e quello C++ sono punti di estensione per adattatori futuri di voce/screen reader, audio spaziale, aptica e visuale. Questa prima implementazione non include NVDA, screen reader, MetaSound, DualSense, focus, sonar o navigazione sonora.

## Limiti e prossimi passi

Non sono inclusi persistenza, `UGameUserSettings`, menu o profili predefiniti. La PR successiva potrà fondare cataloghi separati, revisionabili via CSV/Excel, per oggetti, archetipi di nemici, personaggi, quest, obiettivi, conversazioni, nodi, scelte, condizioni, conseguenze e profili di accessibilità; qui non vengono introdotte Data Table né sistemi di quest, dialogo o combattimento.
