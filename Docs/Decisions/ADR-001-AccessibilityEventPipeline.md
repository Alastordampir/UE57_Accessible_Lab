# ADR-001: pipeline di eventi semantici di accessibilità

- **Stato:** Accettata
- **Decisione:** usare un `UGameInstanceSubsystem` come router di eventi semantici, categorie gerarchiche Gameplay Tags e preferenze runtime applicate prima dei delegate.

## Contesto

Più sistemi di gameplay devono comunicare informazioni accessibili senza dipendere da screen reader, audio, aptica o UI specifici. Le preferenze devono applicarsi coerentemente durante una sessione e supportare regole generali e specializzate.

## Decisione

`UAccessibilitySubsystem` appartiene alla game instance: offre un punto raggiungibile da C++ e Blueprint, ha una durata adatta alla sessione e non richiede un attore globale. Riceve annunci semantici e distribuisce soltanto quelli validi dopo i filtri. I Gameplay Tags modellano una tassonomia estensibile e consentono ereditarietà naturale dalle categorie padre; la categoria più specifica e poi la modalità concreta determinano la regola effettiva.

L'evento dichiara significato e possibili canali, non la tecnologia. Le preferenze vengono applicate prima della distribuzione, così ogni adattatore osserva la stessa decisione, la stessa maschera finale e una sola emissione. `SourceId` evita riferimenti persistenti agli attori.

## Alternative considerate

- Un actor singleton richiederebbe gestione di spawn, level travel e lifetime.
- Un world subsystem legherebbe preferenze e adattatori a ogni world invece che alla sessione.
- Enum piatti per le categorie renderebbero costosi estensione ed ereditarietà.
- Chiamate dirette dal gameplay agli adattatori accoppierebbero contenuto e dispositivi e replicherebbero i filtri.
- Filtrare dentro ogni adattatore potrebbe produrre risultati e conteggi di emissione divergenti.

## Conseguenze

I produttori devono dichiarare categoria, modalità concreta e canali disponibili. La telecamera non può inferire la modalità: un futuro stato autorevole coordinerà esplorazione in prima persona e combattimento tattico in terza persona. Gli adattatori possono iscriversi ai delegate senza modificare il gameplay. Le preferenze si perdono alla fine della sessione; persistenza, menu, profili, tecnologie di output reali e cataloghi CSV/Excel separati restano lavoro futuro.
