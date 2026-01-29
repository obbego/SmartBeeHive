# Feedback Docente – Frontend Alveare

## Analisi dei Requisiti

### Cosa manca / cosa va reso più verificabile (con esempi puntuali)

**Requisiti non funzionali – prestazioni / real-time (verificabilità)**

* Nel documento si fa riferimento a monitoraggio e aggiornamento “in tempo reale”, ma **non sono indicati parametri misurabili**.
* In particolare mancano:

  * frequenza di refresh dei dati (es. ogni *N* secondi)
  * latenza massima accettabile tra ricezione del dato e visualizzazione
  * comportamento atteso in caso di ritardo o assenza dati (indicatore, fallback, avviso).

Indicazione per il gruppo: chiarire la frequenza reale di aggiornamento dei dati (in questo caso caricamento ogni 8 ore su ThingsBoard) e allineare di conseguenza l’esperienza utente. In particolare va specificato:

* ogni quanto il frontend interroga/aggiorna la dashboard (polling o refresh)
* come viene mostrato il “timestamp ultimo aggiornamento”
* quale messaggio/indicatore compare quando i dati sono più vecchi di una soglia (es. “dati non aggiornati”).

Questo rende verificabile il comportamento del frontend rispetto a una telemetria non continua.

**Tracciabilità del requisito (origine e verifica)**

* Non è sempre chiaro se il requisito nasce:

  * da una richiesta del **cliente/committente**
  * oppure da una **scelta progettuale del gruppo backend**.
* Manca inoltre l’indicazione di:

  * chi è responsabile della definizione del requisito
  * chi ne verifica la corretta implementazione (frontend/backend/test).

Indicazione per il gruppo: per i requisiti critici (aggiornamento dati, allarmi, sicurezza) indicare sempre origine del requisito e responsabile della verifica, anche in forma sintetica.

**Allarmi (focus frontend: presentazione e gestione lato utente)**

* Trigger e logica di generazione allarme sono responsabilità backend/sensori; nel documento però manca la parte frontend: **come l’allarme viene gestito dall’utente**.
* Definire almeno:

  * canale lato UI (banner, badge, lista allarmi, evidenza sull’arnia)
  * azione di presa visione/chiusura (“acknowledge”) e relativo stato visibile (es. “aperto / preso in carico / chiuso”)
  * “silenzia” temporaneo (mute) e durata/condizione di riattivazione.

Verifica in test: simulare un allarme generato a backend e verificare corretta visualizzazione, presa visione e comportamento di silenziamento.

## Analisi Funzionale

### Cosa manca / cosa va chiarito (focus frontend)

**Gestione e visualizzazione dello stato di aggiornamento dei dati**

* Poiché i dati non sono continui (caricamento su piattaforma ogni 8 ore), nel documento funzionale manca una descrizione esplicita di come il frontend comunica questa caratteristica all’utente.
* In particolare non è specificato:

  * visualizzazione dell’ultimo timestamp di aggiornamento per ciascuna arnia/sensore;
  * indicatore di “dati non aggiornati” quando l’ultimo aggiornamento supera una soglia temporale;
  * comportamento della dashboard in assenza temporanea di nuovi dati (messaggio informativo, warning, limitazione di alcune viste).

Indicazione per il gruppo: definire questi comportamenti come casi d’uso del frontend e verificarli in test con dataset “vecchi” e con assenza di aggiornamenti.

**Grafici e aggregazioni con telemetria non continua**

* Nel documento funzionale sono previsti grafici e analisi (trend, derivate, FFT, correlazioni), ma manca la definizione di come devono comportarsi quando i dati arrivano a blocchi (ogni 8 ore) e possono avere buchi temporali.
* Mancano in particolare:

  * regole di interpolazione o gestione dei buchi (nessuna interpolazione, linee spezzate, valori nulli);
  * scelta delle finestre temporali predefinite (giorno/settimana/mese) e granularità (oraria/giornaliera);
  * indicazione su quali grafici sono disponibili in modalità “confronto” e quali solo per singola arnia.

Indicazione per il gruppo: descrivere almeno 2–3 casi d’uso completi (vista 7 giorni, confronto arnie, zoom su intervallo) specificando cosa vede l’utente quando mancano punti dati.

**Vista multi-arnia e indicatori di gruppo (mancante)**

* Nel documento funzionale manca una funzionalità di sintesi che permetta di confrontare più arnie contemporaneamente.
* In particolare non sono previsti:

  * una dashboard di overview con mediana (o media) per parametro (T/H/peso) su un intervallo selezionato;
  * indicatori di dispersione (quartili o bande) per evidenziare la variabilità del gruppo;
  * evidenziazione degli outlier (arnie che si discostano significativamente dal gruppo);
  * possibilità di selezionare più arnie e sovrapporre/nascondere le curve.

Indicazione per il gruppo: introdurre una sezione “Confronto arnie” che mostri trend di gruppo e scostamenti, utile per individuare rapidamente anomalie senza aprire ogni arnia singolarmente.

## Analisi Tecnica (valutazione indicativa)


### Cosa manca / cosa va migliorato

**Perimetro ThingsBoard vs backend applicativo**

* Nel documento tecnico non è esplicitato in modo univoco quali responsabilità sono coperte da ThingsBoard e quali da un eventuale backend applicativo.
* Rimane ambiguo:

  * se autenticazione e ruoli sono demandati alla piattaforma o a un servizio dedicato;
  * se le API REST/WebSocket verso il frontend sono quelle native della piattaforma o esposte da un backend ad hoc;
  * dove risiedono entità come utenti, log e arnie (entità di piattaforma vs DB esterno).

Indicazione per il gruppo: chiarire esplicitamente il perimetro tra piattaforma IoT e backend applicativo, distinguendo funzioni native/configurate in piattaforma (device, telemetria, allarmi, dashboard) da quelle che richiedono componenti aggiuntive.

**Dati audio e analisi FFT**
Per la parte audio è necessario chiarire quali dati vengono effettivamente trasmessi (audio grezzo vs feature estratte), dove avviene l’analisi (sensore/centralina/piattaforma) e con quale frequenza. In assenza di queste decisioni il requisito resta ambiguo e non verificabile.

**Allineamento tra invio dati e ‘real time’**
È necessario allineare il concetto di ‘real time’ con la reale frequenza di invio dei dati, chiarendo che l’aggiornamento del frontend avviene a blocchi e indicando come vengono gestiti timestamp e dati non aggiornati.

**Scelta architetturale del frontend**
È opportuno esplicitare se il frontend sarà realizzato principalmente tramite dashboard della piattaforma IoT oppure come webapp personalizzata, così da evitare sovrapposizioni e chiarire il perimetro di sviluppo.

