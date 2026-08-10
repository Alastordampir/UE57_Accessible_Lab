# ADR-002: catalogo CSV/Data Table per gli oggetti accessibili

- **Stato:** Accettata
- **Contesto:** i metadati accessibili degli oggetti devono poter essere revisionati senza duplicare valori nei componenti o richiedere asset binari in questa PR.

## Decisione

Adottiamo un CSV UTF-8 come fonte di authoring testuale e la struttura Unreal `FAccessibleObjectCatalogRow`, derivata da `FTableRowBase`, come schema della Data Table. La colonna iniziale `Name` diventa il RowName e unico identificativo stabile. Le altre colonne sono `AccessibleName`, `AccessibleDescription`, `Category`, `NavigationPriority` e `bAccessibilityEnabled`.

La struttura valida deterministicamente ogni riga. Nome e descrizione non possono essere entrambi vuoti o composti soltanto da spazi; la priorità deve essere fra 0 e 100; la categoria deve essere una fra `Objective`, `Interactable`, `Enemy`, `Ally`, `Hazard` e `Landmark`. `Unknown` e valori enum non riconosciuti sono invalidi, mentre `bAccessibilityEnabled=false` è valido.

Il componente applica una riga soltanto dopo la validazione completa. Anche la risoluzione di un `FDataTableRowHandle` converge sullo stesso metodo: tabella nulla, RowName nullo o assente e riga invalida falliscono senza mutare parzialmente il componente.

## Motivazione

Il CSV è leggibile con Excel e con editor testuali, produce diff comprensibili e consente review da parte di persone che non usano Unreal Editor. La Data Table offre uno schema riflesso e Blueprint-friendly coerente con Unreal. Questa combinazione separa l'authoring revisionabile dal consumo tipizzato, senza aggiungere nuove dipendenze al modulo.

## Conseguenze e limiti

- Il percorso dati è `CSV → Data Table → componente → pipeline semantica`.
- L'author deve preservare UTF-8, intestazioni e RowName stabili.
- Il CSV dimostrativo è importato in memoria dagli automation test; non viene caricato come file grezzo a runtime.
- Non sono presenti asset `.uasset` né è richiesta un'importazione manuale tramite Editor.
- I cinque valori sono dati statici; preferenze, modalità, canali, stato di gameplay e invio dei delegate restano runtime.
- L'uso di `FText` non costituisce ancora la futura pipeline completa di localizzazione, che richiederà una decisione e strumenti successivi.
- Quest, dialogo, combattimento, persistenza e adattatori reali restano fuori da questa decisione.
