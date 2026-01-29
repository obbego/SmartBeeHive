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

**Verifica tecnica end-to-end**
Si consiglia di includere almeno uno scenario di test tecnico end-to-end che descriva condizioni iniziali, flusso e risultato atteso, così da verificare l’integrazione complessiva dell’architettura.
