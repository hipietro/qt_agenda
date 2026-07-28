# Relazione progetto Programmazione a Oggetti

**Autore:** Pietro Leonardo Acampora  
**Matricola:** 2146762  
**Titolo:** Agenda Qt  
*Applicazione desktop per la gestione di attività personali*

## Introduzione

Agenda Qt è un'applicazione desktop sviluppata interamente in C++17 con Qt Widgets. Il programma organizza attività personali eterogenee - eventi, scadenze, promemoria e checklist - attraverso una gerarchia di classi, un'interfaccia grafica a finestra singola e persistenza locale in JSON. L'utente può creare, cercare, visualizzare, modificare, eliminare e completare le attività, lavorando sia sui campi comuni sia sugli attributi specifici di ogni tipo.

Il progetto è stato svolto individualmente. L'obiettivo architetturale principale è mantenere il modello logico indipendente dai widget e usare il polimorfismo per operazioni realmente differenti, non soltanto per getter o etichette. Per questo sono stati applicati Visitor per visualizzazione, modifica e serializzazione, Command per le azioni reversibili e una factory registry per ricostruire gli oggetti dal JSON.

## Conformità ai vincoli obbligatori

| N. | Vincolo | Evidenza nel progetto |
| ---: | --- | --- |
| 1 | Lavoro individuale e originale | Repository, struttura, funzionalità e documentazione appartengono al singolo autore e sono specifici di Agenda Qt. |
| 2 | Implementazione in C++ | Tutta la logica applicativa è in file `.h` e `.cpp` C++17. QSS, JSON e risorse Qt sono dati o presentazione. |
| 3 | GUI in Qt | `MainWindow` deriva da `QMainWindow` e usa Qt Widgets, form, liste, stacked pages, menu, file dialog e overview mensile. |
| 4 | Build nel Docker fornito | Il progetto usa qmake e il Dockerfile Ubuntu 24.04/Qt 6 del corso; la validazione finale usa `qmake6` e `make`. |
| 5 | Incapsulamento e singolo concetto | Stato privato, accesso tramite metodi e classi separate per manager, storage, ricerca, filtri, rendering, comandi e controller. |
| 6 | Separazione modello/GUI | `src/model` non include Qt Widgets; la GUI dipende dal modello, non il contrario. |
| 7 | Robustezza | Validazione dei form, controlli su puntatori e id, gestione errori I/O, conferma modifiche non salvate e test automatici. |
| 8 | Polimorfismo non banale | Visitor con doppio dispatch e Command con `execute()`/`undo()` dinamici e comportamenti differenti. |
| 9 | Nessun getType per il flusso | `ActivityKind` è usato soltanto per filtro, selezione e costruzione iniziale; Visitor, Command e factory gestiscono il comportamento. |
| 10 | Almeno tre classi concrete | Sono presenti quattro tipi con dati, validazioni, rendering e persistenza differenti. |
| 11 | CRUD, ricerca e campi specifici via GUI | Tutti i workflow sono disponibili nella GUI; creazione e modifica mostrano form diversi per tipo. |
| 12 | Persistenza strutturata locale | JSON completo di attività, categorie, ricorrenze, checklist e template. |
| 13 | Save/Load con dialog | `QFileDialog` a runtime, annullamento sicuro e nessun percorso hardcoded. |
| 14 | Navigazione nella stessa finestra | `ActivityCreationPage` e `ActivityEditPage` sono `QWidget` nello stack della `MainWindow`. |
| 15 | Relazione conforme | Relazione italiana, massimo 8 pagine, corpo 10 pt e tutte le sezioni richieste. |

## Descrizione del modello logico

La classe astratta `Activity` rappresenta il concetto comune di attività e mantiene privati identificatore, titolo, descrizione, categoria, priorità, completamento, timestamp e regola di ricorrenza. Espone `accept(ActivityVisitor&)` per il doppio dispatch e operazioni virtuali quali `primaryDate()`, `isOverdue()` e `clone()`. `ActivityManager` possiede gli oggetti tramite `std::unique_ptr`, rendendo esplicita l'ownership ed evitando gestione manuale della memoria.

| Classe concreta | Attributi specifici | Comportamento significativo |
| --- | --- | --- |
| `EventActivity` | inizio/fine, luogo, partecipanti | validazione fine > inizio; card e dettaglio con durata e intervallo |
| `DeadlineActivity` | data limite, contesto, `hardDeadline` | scadenza puntuale e indicazione della rigidità |
| `ReminderActivity` | data/ora, anticipo, nota | configurazione e visualizzazione dell'anticipo |
| `ChecklistActivity` | data obiettivo, elementi | progresso percentuale e completamento derivato dagli item |

I diagrammi aggiornati sono disponibili in [`docs/uml/README.md`](../uml/README.md).

## Struttura e separazione delle responsabilità

Il codice è diviso in quattro aree. Il modello contiene gerarchia `Activity`, manager, ricerca, filtri, categorie, ricorrenze e template; la GUI costruisce i widget e coordina i workflow; la persistenza converte lo stato in JSON; i comandi rappresentano modifiche reversibili. `MainWindow` coordina segnali, azioni e navigazione, ma delega le operazioni specifiche a classi dedicate.

| Componente | Responsabilità |
| --- | --- |
| `ActivityManager` | possiede la collezione polimorfica; aggiunge, sostituisce, rimuove e cerca per id |
| `SearchEngine` / `ActivityFilter` | ricerca normalizzata e fuzzy; filtri combinabili e ordinamenti |
| `CategoryManager` | valida nomi e colori, evita duplicati e gestisce aggiornamento/rimozione |
| `ActivityTemplateManager` | gestisce prototipi nominati e crea attività tramite `cloneWithNewId()` |
| `AgendaJsonStorage` | salva e carica agenda, categorie e template; restituisce errori dettagliati |
| Controller e Visitor GUI | separano interazioni, rendering e form di modifica da `MainWindow` |

## Polimorfismo non banale

Il meccanismo principale è il Visitor pattern. `Activity::accept(ActivityVisitor&)` è astratto; ogni sottoclasse richiama l'overload `visit(...)` corrispondente al proprio tipo. Il comportamento concreto viene quindi scelto tramite doppio dispatch senza interrogare un codice di tipo e senza concentrare una catena di `if`/`switch` nella GUI o nella persistenza.

| Visitor | Operazione dinamica | Valore aggiunto |
| --- | --- | --- |
| `ActivityListItemVisitor` | crea card diverse | icona, colore, intervallo e luogo, deadline e rigidità, anticipo o avanzamento checklist |
| `ActivityDetailVisitor` | costruisce sezioni diverse | mostra tutti i campi specifici senza downcast nella `MainWindow` |
| `ActivityEditFormVisitor` | Populate / Validate / Build | seleziona il form, verifica regole specifiche e ricostruisce il sottotipo |
| `ActivityJsonSerializationVisitor` | serializza campi differenti | produce il JSON specifico dei quattro tipi |

Ogni overload realizza una procedura profondamente diversa: crea widget differenti, applica regole di validazione diverse, costruisce oggetti di classi diverse o salva strutture JSON diverse. L'aggiunta di un tipo obbliga il compilatore a segnalare i Visitor da estendere.

Anche il sistema undo/redo usa polimorfismo. `Command` dichiara `execute()`, `undo()`, `description()`, `undoDescription()` e `redoDescription()`. `AddActivityCommand`, `RemoveActivityCommand`, `UpdateActivityCommand` e `ToggleCompletionCommand` conservano stati differenti e implementano procedure inverse differenti. `CommandHistory` usa due stack di `std::unique_ptr<Command>` e trasferisce la proprietà durante undo e redo.

## Uso controllato del tipo

`ActivityKind` rimane un classificatore. È impiegato come dato del filtro, come valore scelto dall'utente nel form di creazione e nel punto in cui deve essere costruito un oggetto che ancora non esiste. Rendering, dettaglio, modifica e serializzazione di attività esistenti passano dai Visitor; la deserializzazione passa da `ActivityFactoryRegistry`. Non esiste quindi controllo di flusso basato su `getType`/`kind` in sostituzione del polimorfismo.

## Persistenza dei dati

La persistenza locale usa JSON. `AgendaJsonStorage` coordina salvataggio e caricamento di attività, categorie e template. `ActivityJsonSerializationVisitor` scrive i campi comuni e specifici; in lettura `ActivityFactoryRegistry` associa l'identificatore testuale del formato a una funzione factory, evitando uno switch centrale.

```json
{
  "version": 1,
  "categories": [ ... ],
  "activities": [ { "type": "event", ... } ],
  "templates": [ ... ]
}
```

Le date sono salvate in ISO 8601 con millisecondi. Il caricamento valida struttura, campi obbligatori e tipo registrato prima di modificare i manager. Save, Save As e Load usano `QFileDialog`; Save riutilizza soltanto un percorso scelto dall'utente.

## Interfaccia grafica e navigazione

`MainWindow` possiede un `QStackedWidget` denominato `m_workspaceStack`. La pagina di agenda mostra lista, dettaglio e panoramica mensile; `ActivityCreationPage` e `ActivityEditPage` sono `QWidget` nello stesso stack. Add apre la pagina di creazione, Edit o doppio click apre la modifica, Create/Save applicano un Command e ritornano all'agenda, Cancel ritorna senza modificare il modello. Le sole modali sono utilità ammesse: file dialog, conferme, messaggi e gestione categorie.

| Workflow | Implementazione |
| --- | --- |
| Creazione | form comune e stack per campi specifici; validazione; `AddActivityCommand` |
| Visualizzazione | card e dettaglio costruiti dai Visitor |
| Modifica | `ActivityEditFormVisitor` mantiene sottotipo, id e campi specifici; `UpdateActivityCommand` |
| Eliminazione | pulsante o menu contestuale, conferma e `RemoveActivityCommand` |
| Lista | click singolo seleziona, doppio click modifica, click destro opera sull'elemento puntato |

### Scorciatoie e interazioni dirette

Le azioni espongono scorciatoie native Qt: New (`Ctrl/Cmd+N`), Edit (`Ctrl+E`), Find (`Ctrl/Cmd+F`), Open (`Ctrl/Cmd+O`), Save (`Ctrl/Cmd+S`), Save As (`Ctrl/Cmd+Shift+S`), template (`Ctrl+T` e `Ctrl+Shift+T`), categorie (`Ctrl+Shift+C`) e Undo/Redo secondo la convenzione del sistema operativo. Con la lista focalizzata, Enter modifica, Space cambia completamento e Delete/Backspace elimina. Nei form Enter conferma, `Ctrl/Cmd+Enter` conferma dai campi multilinea ed Esc annulla. Mouse e trackpad supportano selezione singola, doppio click e click secondario con menu contestuale applicato all'elemento puntato.

## Funzionalità aggiuntive

### Ricerca normalizzata, pesata e tollerante agli errori

`SearchEngine` normalizza query e campi con Unicode Normalization Form D, conversione in minuscolo, semplificazione degli spazi e rimozione dei segni diacritici. Titolo, categoria, descrizione e `summary()` ricevono punteggi diversi per match esatto, prefisso o contenimento; il titolo ha priorità maggiore. I risultati sono ordinati per punteggio e poi alfabeticamente.

Quando non esistono corrispondenze dirette, viene calcolata la distanza di Levenshtein sui campi completi e sulle parole di almeno tre caratteri. La soglia è 1 per query fino a 4 caratteri, 2 fino a 7 e 3 per query più lunghe.

### Filtri e ordinamenti

`ActivityFilter` applica tipo, categoria, priorità, ricorrenza, completamento, overdue e intervallo temporale. Per `isOverdue()` e `primaryDate()` usa dispatch virtuale. L'ordinamento supporta data, titolo, priorità, completamento, `createdAt` e `updatedAt`, con titolo e id come tie-break deterministico. La ricerca viene applicata dopo i filtri.

### Categorie, ricorrenze e template

`CategoryManager` rifiuta nomi vuoti, duplicati case-insensitive e colori diversi da `#RRGGBB`. Una rinomina aggiorna le attività interessate. `RecurrenceRule` supporta frequenze giornaliera, settimanale, mensile e annuale, intervallo e fine mai/entro data/dopo N occorrenze, con limite di 10.000 iterazioni di sicurezza.

Un template conserva un clone polimorfico completo dell'attività. Il riuso chiama `cloneWithNewId()`: il sottotipo e tutti i dati vengono mantenuti, ma la nuova attività riceve un'identità distinta. Categorie e template sono persistiti insieme all'agenda.

### Ulteriori miglioramenti

- panoramica mensile con filtro rapido per giorno e controlli Today/mese dedicati;
- splitter ridimensionabili, palette coerente e iconografia specifica per tipi e azioni;
- marker di modifiche non salvate e conferma prima di operazioni distruttive;
- feedback I/O con nome file, percorso, conteggi e motivazione dell'errore;
- scorciatoie standard, navigazione da tastiera e layout responsive;
- menu contestuale portabile per mouse e trackpad;
- file `examples/sample_agenda.json` con tutti i tipi e le funzioni persistenti.

## Testing, qualità e robustezza

La suite core copre doppio dispatch, renderer, serializzazione/deserializzazione, round trip dell'agenda, JSON malformato, filtri, ricerca e sequenze ripetute di undo/redo. La suite GUI istanzia la vera `MainWindow` e controlla pagine interne, doppio click e menu contestuale. In CI i test sono compilati su Ubuntu 24.04 con Qt 6.4.2 sotto Xvfb; gli stessi target sono stati eseguiti su macOS ARM con Qt 6.10.1.

| Area | Controllo |
| --- | --- |
| Visitor | quattro overload e rendering specifico |
| Persistenza | tutti i campi comuni e specifici, categorie, template, ricorrenze e checklist |
| Input non valido | tipo sconosciuto e JSON malformato falliscono senza corrompere lo stato |
| Command | execute, undo e redo ripetuti; ramo redo cancellato dopo un nuovo comando |
| GUI | pagine interne, nessuna modale attiva, interazioni mouse corrette |
| Audit statico | blocca Qt Widgets nel modello, dialog obsoleti e dispatch improprio tramite `kind()` |

L'efficienza è adeguata alla scala di un'agenda personale: collezioni con vector e smart pointer, filtri lineari seguiti dall'ordinamento richiesto, Levenshtein con due sole righe della matrice e limite di sicurezza per le ricorrenze.

## Rendicontazione delle ore

| Attività | Ore previste | Ore effettive |
| --- | ---: | ---: |
| Analisi dei requisiti e progettazione | 7 | 10 |
| Modello logico, gerarchia e manager | 10 | 14 |
| Visitor, factory e polimorfismo | 6 | 11 |
| Interfaccia Qt e navigazione interna | 11 | 17 |
| Persistenza JSON e file di esempio | 5 | 7 |
| Ricerca, filtri, categorie, ricorrenze e template | 6 | 11 |
| Command, test, debug e CI/Docker | 4 | 8 |
| README, UML e relazione | 1 | 3 |
| **Totale** | **50** | **81** |

## Modifiche rispetto alla consegna precedente

Per la riconsegna sono state introdotte modifiche sostanziali al progetto: Visitor per lista, dettaglio, modifica e serializzazione; `ActivityFactoryRegistry` per il caricamento; gerarchia Command completa con undo/redo; pagine di creazione e modifica integrate nella `MainWindow`; controller dedicati alle interazioni della lista; test architetturali e GUI; audit statico in CI. Sono stati inoltre consolidati precisione temporale del JSON, gestione degli errori, documentazione, diagrammi UML e presentazione grafica. Le funzionalità utente già presenti sono state mantenute e verificate contro la nuova architettura.

## Compilazione

```bash
mkdir -p build
cd build
qmake6 ../agenda_qt.pro
make -j$(nproc)
```

Nel container del corso:

```bash
docker build -t unipd-oop/qt-env:2025 .
docker run --rm -it -v "$PWD":/workspace -w /workspace unipd-oop/qt-env:2025 bash
mkdir -p build_docker && cd build_docker
qmake6 ../agenda_qt.pro && make -j$(nproc)
```

## Conclusione

Agenda Qt soddisfa i vincoli obbligatori mediante una gerarchia di quattro attività realmente differenti, separazione tra modello e GUI, navigazione a finestra singola, persistenza JSON scelta a runtime e polimorfismo non banale basato su Visitor e Command. Le funzionalità aggiuntive - ricerca normalizzata e fuzzy, filtri e ordinamenti, categorie, ricorrenze, template, overview mensile e undo/redo - sono integrate nel modello, persistite e coperte dai test.
