# Relazione progetto Programmazione a Oggetti

**Autore:** Pietro Leonardo Acampora  
**Matricola:** 2146762  
**Titolo:** Agenda Qt  
*Applicazione desktop per la gestione di attività personali*

## Introduzione

Agenda Qt è un'applicazione desktop sviluppata interamente in C++17 con Qt Widgets. Permette di creare, modificare, visualizzare, ricercare, filtrare, ordinare ed eliminare attività personali, salvando lo stato dell'agenda in file JSON scelti dall'utente. Il progetto è stato sviluppato come lavoro individuale per applicare in modo concreto ereditarietà, classi astratte, incapsulamento, polimorfismo dinamico e separazione tra modello logico e interfaccia grafica.

**Dichiarazione di originalità.** Dichiaro che il progetto, il codice sorgente e la presente relazione sono frutto del mio lavoro individuale originale. Le librerie impiegate sono quelle standard di C++ e Qt; eventuali strumenti di supporto sono stati usati esclusivamente per compilazione, verifica e documentazione.

L'utente può gestire quattro tipologie concrete: eventi, scadenze, promemoria e checklist. Tutte condividono titolo, descrizione, categoria, priorità, completamento, timestamp e ricorrenza, ma possiedono dati e regole differenti. Le funzionalità aggiuntive comprendono categorie personalizzate, template, ricorrenze, calendario mensile, undo/redo, ricerca, filtri combinabili e menu contestuale.

## Descrizione del modello

Il modello è centrato sulla classe astratta `Activity`. I dati comuni sono privati e vengono modificati tramite metodi pubblici controllati; le sottoclassi conservano soltanto lo stato specifico del proprio concetto. `ActivityManager` possiede le attività mediante `std::unique_ptr`, rendendo esplicita l'ownership ed evitando cancellazioni manuali. `CategoryManager` e `ActivityTemplateManager` gestiscono rispettivamente categorie e prototipi riutilizzabili.

Le quattro classi concrete sono:

- `EventActivity`: inizio, fine, luogo e partecipanti;
- `DeadlineActivity`: data limite, contesto e flag di scadenza rigida;
- `ReminderActivity`: data/ora, anticipo e nota del promemoria;
- `ChecklistActivity`: data obiettivo, elementi, progresso e completamento derivato.

I diagrammi aggiornati sono disponibili in [`docs/uml/README.md`](../uml/README.md).

## Struttura e responsabilità

Le classi sono organizzate in quattro aree principali. Il modello contiene il dominio e i servizi di ricerca e filtraggio; la GUI contiene widget e controller; la persistenza converte e ricostruisce gli oggetti; i comandi rappresentano modifiche reversibili. Questa separazione impedisce ai widget di diventare il luogo in cui risiedono regole di dominio e gestione dello stato.

| Componente | Responsabilità principale |
| --- | --- |
| `Activity` e sottoclassi | dati comuni e comportamento specifico delle attività |
| `ActivityManager` | ownership, aggiunta, rimozione, aggiornamento e ricerca per id |
| `SearchEngine` / `ActivityFilter` | ricerca testuale, filtri e ordinamenti senza dipendenze GUI |
| `CategoryManager` / `ActivityTemplateManager` | categorie coerenti e creazione da prototipi polimorfici |
| GUI visitors e presentation controller | costruzione di lista e dettagli in base al tipo dinamico |
| `AgendaJsonStorage` / factory registry | salvataggio e ricostruzione dell'agenda completa |
| `CommandHistory` e comandi concreti | execute, undo e redo delle operazioni sul modello |

Il modello usa tipi di Qt Core come `QString`, `QDateTime` e `QVector`, ma non include classi Qt Widgets. `ActivityVisitor` appartiene al modello e dichiara soltanto overload sui tipi concreti; i visitor grafici dipendono dal modello, mai il contrario.

## Polimorfismo non banale

Il requisito principale non è dimostrato da semplici getter virtuali. Il meccanismo centrale è il Visitor pattern: ogni `Activity` concreta implementa `accept(ActivityVisitor&)`, che richiama l'overload `visit` corrispondente al proprio tipo. Si ottiene così un doppio dispatch: il codice chiamante lavora con `Activity*`, mentre il comportamento eseguito dipende sia dal visitor scelto sia dal tipo dinamico dell'attività.

| Visitor | Comportamento dinamico |
| --- | --- |
| `ActivityListItemVisitor` | costruisce card differenti per eventi, scadenze, promemoria e checklist |
| `ActivityDetailVisitor` | genera sezioni specifiche: durata, stato scadenza, anticipo o progresso |
| `ActivityEditFormVisitor` | popola, valida e ricostruisce il form corretto senza switch sul tipo esistente |
| `ActivityJsonSerializationVisitor` | scrive nel JSON campi specifici diversi per ogni sottoclasse |

Un secondo esempio è la gerarchia `Command`. `CommandHistory` conserva puntatori all'interfaccia astratta `Command` e invoca `execute()` e `undo()` senza conoscere il comando concreto. `AddActivityCommand`, `RemoveActivityCommand`, `UpdateActivityCommand` e `ToggleCompletionCommand` memorizzano e ripristinano stati differenti.

Metodi come `primaryDate()`, `isOverdue()` e `clone()` restano utili per ordinamento, scadenze e duplicazione polimorfica, ma non sono più presentati come unica prova del requisito: il valore aggiunto principale deriva dai visitor e dai comandi.

## Uso controllato del tipo

`ActivityKind` è mantenuto come classificatore descrittivo. Viene usato per il filtro scelto dall'utente e nella pagina di creazione, dove una scelta esplicita deve determinare quale oggetto costruire. Non viene usato per decidere come visualizzare, modificare o serializzare un'attività già esistente.

La deserializzazione non contiene una catena centrale di `if` o `switch`: `ActivityFactoryRegistry` associa l'identificatore testuale del JSON a una factory. L'aggiunta di un nuovo tipo richiede la registrazione della nuova factory e dei relativi overload del visitor, senza modificare un unico blocco di dispatch sparso nella GUI.

## Persistenza dei dati

`AgendaJsonStorage` coordina salvataggio e caricamento dell'intero stato. La serializzazione comune conserva id, dati descrittivi, priorità, completamento, timestamp con precisione al millisecondo e ricorrenza; il visitor aggiunge i campi specifici. Durante il caricamento la factory ricostruisce la sottoclasse corretta e segnala tipi sconosciuti o dati malformati.

```json
{
  "version": 1,
  "categories": [ ... ],
  "activities": [ ... ],
  "templates": [ ... ]
}
```

Save, Save As e Load sono disponibili a runtime dal menu File. `QFileDialog` permette di scegliere il percorso; non sono presenti path hardcoded. Gli errori di apertura, parsing o scrittura vengono restituiti dalla persistenza e mostrati all'utente.

## Interfaccia grafica e navigazione

`MainWindow` contiene la lista, i filtri, il pannello dettagli e un `QStackedWidget` denominato `m_workspaceStack`. `ActivityCreationPage` e `ActivityEditPage` derivano da `QWidget` e sono inserite direttamente nello stack: creazione e modifica avvengono quindi nella stessa finestra principale. Salvataggio e annullamento riportano alla pagina dei dettagli.

La pagina di creazione presenta campi comuni e uno stack di campi specifici. La pagina di modifica mantiene il tipo concreto originale e usa `ActivityEditFormVisitor` per selezionare la sezione corretta, validare i dati e produrre il nuovo oggetto. Il dialog modale rimasto riguarda soltanto la gestione delle categorie e non sostituisce i workflow principali richiesti.

## Funzionalità implementate

- creazione, visualizzazione, modifica, eliminazione e completamento di quattro tipi concreti;
- ricerca case-insensitive e parziale, filtri combinabili e ordinamenti multipli;
- categorie personalizzate, template, ricorrenze e calendario mensile;
- undo/redo tramite Command pattern e tracciamento delle modifiche non salvate;
- salvataggio e caricamento JSON tramite file dialog, con file di esempio;
- singolo click per selezione, doppio click per modifica e menu contestuale sulla voce puntata;
- feedback visivo per priorità, scadenza, completamento e progresso checklist.

## Testing e validazione

Il progetto include due target Qt Test separati. La suite core verifica il modello e l'architettura; la suite GUI usa la vera `MainWindow` e un display virtuale Xvfb su Linux. I test sono eseguiti automaticamente da GitHub Actions.

| Area | Evidenza automatica |
| --- | --- |
| Visitor | double dispatch e rendering specifico per tutti e quattro i tipi |
| Persistenza | round trip completo, campi specifici, categorie, template, ricorrenza e input malformato |
| Command | execute, undo, redo ripetuti e cancellazione del ramo redo |
| Ricerca e filtri | ricerca, combinazioni di criteri e ordinamenti |
| GUI | pagine interne, singolo/doppio click, menu contestuale e spazio vuoto |

Risultati verificati: 9 test core superati e 3 test GUI superati sia su Ubuntu 24.04 con Qt 6.4.2 sia su macOS Apple Silicon con Qt 6.10.1. La CI controlla anche che non ricompaiano dipendenze Qt Widgets nel modello, vecchi dialog di creazione/modifica o dispatch basato su `kind()` per attività esistenti.

## Modifiche rispetto alla consegna precedente

La revisione successiva alla prima valutazione non si è limitata alla documentazione. Sono stati modificati i punti architetturali che rendevano insufficienti i requisiti sul polimorfismo e sulla navigazione:

- introdotta l'interfaccia `ActivityVisitor` e `accept()` nelle quattro sottoclassi;
- sostituito il dispatch grafico basato sul tipo con visitor dedicati per lista e dettagli;
- spostata la logica specifica del form di modifica in `ActivityEditFormVisitor`;
- introdotto `ActivityJsonSerializationVisitor` e un `ActivityFactoryRegistry`;
- limitato `ActivityKind` ai casi giustificati di filtro, selezione e costruzione;
- sostituiti i vecchi dialog di creazione e modifica con pagine interne a `MainWindow`;
- rafforzata la gerarchia Command e la cronologia undo/redo;
- aggiunti test architetturali, di regressione e GUI con esecuzione continua in CI;
- aggiunti controlli statici contro regressioni sui requisiti 8, 9 e 14;
- aggiornati UML, README e relazione per descrivere il codice effettivamente consegnato.

Nella relazione precedente `primaryDate()`, `isOverdue()` e `clone()` erano presentati come principali esempi di polimorfismo e la GUI era descritta con dialog dedicati. La versione attuale dimostra invece comportamento dinamico strutturale tramite Visitor e Command e ospita i workflow principali nello stesso `MainWindow`.

## Rendicontazione ore

Le ore sono una stima aggiornata. Le 57 ore della prima consegna sono state integrate con il lavoro di revisione architetturale, test e documentazione svolto per la riconsegna.

| Attività | Prima consegna | Revisione | Totale |
| --- | ---: | ---: | ---: |
| Analisi e progettazione | 8 | 1 | 9 |
| Modello logico e polimorfismo | 11 | 7 | 18 |
| Interfaccia grafica Qt | 15 | 5 | 20 |
| Persistenza JSON | 6 | 2 | 8 |
| Ricerca, filtri, categorie, ricorrenze e template | 7 | 0 | 7 |
| Undo/redo e rifiniture | 4 | 1 | 5 |
| Testing, debug, CI e validazione | 4 | 5 | 9 |
| README, UML e relazione | 2 | 3 | 5 |
| **Totale** | **57** | **24** | **81** |

## Compilazione

```bash
mkdir -p build
cd build
qmake6 ../agenda_qt.pro
make -j$(nproc)
```

Per l'ambiente standard del corso:

```bash
docker build -t unipd-oop/qt-env:2025 .
docker run --rm -it -v "$PWD":/workspace -w /workspace unipd-oop/qt-env:2025 bash
mkdir -p build_docker && cd build_docker
qmake6 ../agenda_qt.pro
make -j$(nproc)
```

## Conclusione

Agenda Qt realizza un'applicazione desktop completa e coerente con gli obiettivi del corso. Il modello contiene quattro concetti concreti significativamente differenti, la GUI permette tutti i workflow richiesti e la persistenza conserva l'intera agenda in formato strutturato.

La revisione architetturale ha reso esplicito il valore del polimorfismo: i visitor selezionano comportamento grafico, di editing e di serializzazione in base al tipo dinamico, mentre la gerarchia Command incapsula operazioni reversibili differenti. Il tipo enumerato non sostituisce questi meccanismi e creazione e modifica restano all'interno della finestra principale.
