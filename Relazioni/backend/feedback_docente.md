# Feedback Docente – Backend Alveare

## Analisi dei Requisiti

Nel backend alcuni requisiti di progetto risultano descritti solo implicitamente all’interno dell’analisi tecnica. In particolare, andrebbero esplicitati come requisiti: la gestione di dati inviati in modo non continuo, la necessità di persistenza e consultazione dello storico, la gestione degli allarmi come funzionalità di sistema e il supporto a più arnie. Attualmente questi aspetti emergono come conseguenze della soluzione tecnica adottata, ma meritano di essere formalizzati come requisiti per rendere più chiara la fase di verifica.

Nel documento non è inoltre presente una distinzione esplicita tra requisiti core e funzionalità di estensione. La priorità dei requisiti può essere dedotta solo dalla complessità delle soluzioni tecniche proposte, ma non è dichiarata come scelta progettuale.

Infine, non è trattato in modo esplicito il tema della verifica dei requisiti: manca l’indicazione di quali requisiti verranno effettivamente verificati e con quali criteri minimi di accettazione. Esplicitare questi aspetti migliorerebbe la chiarezza del progetto e la gestione delle fasi di test.

## Analisi Funzionale

Nei documenti backend la parte funzionale è spesso assorbita nella descrizione tecnica della soluzione. Manca una formalizzazione sintetica dei principali casi d’uso del backend.

In particolare non sono esplicitati:

* i servizi/funzioni che il backend eroga in termini di comportamento;
* gli eventi e i risultati prodotti (dati storici, allarmi, log, stati);
* gli attori che interagiscono con tali funzioni (frontend, piattaforma, scheduler esterno, dispositivi).

Si consiglia di esplicitare i principali casi d’uso del backend rendendoli verificabili, separandoli dalla descrizione delle componenti tecniche.

È inoltre consigliato, a fini didattici, inserire un riferimento esplicito al ciclo funzionale del dato (acquisizione, validazione, storicizzazione e disponibilità), non per vincolare l’implementazione ma per chiarire le responsabilità del backend rispetto alla consistenza del dato.

## Analisi Tecnica (valutazione indicativa)

**Protocollo LoRa “Niagara” (perimetro e validazione)**

* Il protocollo è descritto con buon livello tecnico, ma manca una chiarificazione di perimetro e assunzioni operative.
* In particolare non sono esplicitati:

  * quali parti del protocollo sono indispensabili nella prima iterazione e quali sono estensioni;
  * le assunzioni operative (frequenza invio, dimensione messaggi, numero nodi, tasso errore atteso);
  * criteri minimi di validazione/test (condizioni e metriche di accettazione).

Indicazione per il gruppo: esplicitare cosa è core nella prima iterazione e definire criteri minimi di verifica tecnica del protocollo, così da rendere sostenibile la fase di test.

**Dipendenze critiche e meccanismi di fallback**
Poiché l’architettura introduce componenti critiche (scheduler esterno e RPC), va esplicitato cosa accade in caso di indisponibilità o fallimento di tali componenti e quali meccanismi minimi di fallback e logging sono previsti, così da rendere verificabile l’affidabilità complessiva del backend.

15/03/2026
# Feedback Docente – Gantt e Project Plan

## Diagramma di Gantt

Il diagramma di Gantt risulta coerente con la pianificazione richiesta e rappresenta in modo chiaro le principali fasi del progetto. Le attività sono organizzate secondo una sequenza temporale plausibile e allineata con le milestone indicate nella consegna. È positivo anche il fatto che il Gantt evidenzi l’evoluzione del lavoro dopo i feedback ricevuti, mostrando una pianificazione realistica delle attività di sviluppo, integrazione e verifica.

Nel complesso il Gantt può essere considerato uno strumento utile di pianificazione del lavoro. Un possibile miglioramento potrebbe riguardare un maggiore dettaglio di alcune attività tecniche intermedie, in modo da rendere ancora più evidente il progresso del progetto tra una milestone e l’altra.

## Project Plan

Il Project Plan risulta ben impostato e mostra una buona comprensione del significato di questo documento. Sono presenti gli elementi principali richiesti: obiettivi del progetto, ambito di lavoro, architettura del sistema, milestone e criteri di verifica.

Rispetto alla versione precedente si osserva un miglioramento nella definizione dei ruoli e delle responsabilità del gruppo. Per rendere il documento ancora più efficace come strumento di gestione del progetto, sarebbe utile esplicitare con maggiore dettaglio alcune responsabilità operative durante lo sviluppo, ad esempio per quanto riguarda attività di test, integrazione tra componenti e gestione della documentazione.

Nel complesso il Project Plan è coerente con la consegna assegnata e rappresenta una buona base per la gestione e il monitoraggio dell’avanzamento del progetto.
