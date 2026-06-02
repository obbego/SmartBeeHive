# Analisi Tecnica — Smart Hive Dashboard
*Basaglia Noemi – Bego Francesco – Zara Luigi*

---

## Architettura del Sistema

Il sistema adotta un'architettura a **quattro livelli**:

1. **Sensori e Gateway IoT** — acquisizione dati dalle arnie
2. **Piattaforma IoT (ThingsBoard)** — ricezione, storicizzazione e gestione allarmi
3. **Backend Applicativo (PHP)** — livello intermedio tra ThingsBoard e il browser
4. **Frontend Web (browser)** — interfaccia utente

Il backend PHP non sostituisce ThingsBoard, ma funge da proxy autenticato: espone al frontend un'unica API interna (`api.php`) che aggrega telemetria e allarmi, nascondendo le credenziali ThingsBoard lato client.

---

## Sensori e Gateway IoT

Per ogni arnia sono installati i seguenti sensori:

- temperatura interna;
- umidità relativa;
- peso (per il calcolo della produzione di miele);
- frequenza sonora (microfono con membrana protetta; il dato trasmesso è la **frequenza di picco già estratta** dal dispositivo, non l'audio grezzo).

### Protocollo LoRa Custom

La comunicazione tra i sensori delle arnie e il gateway IoT avviene tramite un **protocollo LoRa personalizzato**, sviluppato dal gruppo hardware del progetto. Il protocollo standard LoRaWAN non era adatto alle esigenze specifiche del sistema, quindi è stato **riscritto da zero** e adattato ai requisiti del progetto.

Il gateway raccoglie i dati inviati tramite questo protocollo LoRa Custom, ne verifica la correttezza e li trasmette a ThingsBoard tramite MQTT (protocollo principale) o HTTP (alternativo). I dati vengono inviati **a blocchi circa ogni 8 ore**, salvo eventi critici che possono innescare un invio immediato.

---

## Backend / Piattaforma IoT (ThingsBoard)

### Istanza utilizzata durante lo sviluppo

Durante lo sviluppo del progetto, ThingsBoard è stato ospitato su **`eu.thingsboard.cloud`** (istanza cloud pubblica). Questa scelta è stata dettata da una necessità pratica: un secondo gruppo stava sviluppando in parallelo il server ThingsBoard fisico della scuola, che non era ancora disponibile nella fase iniziale del progetto. Per non bloccare lo sviluppo del frontend, è stata usata temporaneamente l'istanza cloud.

Non appena il server fisico scolastico è stato reso disponibile dall'altro gruppo, il progetto è passato a utilizzarlo. Il passaggio da un'istanza all'altra richiede la modifica di **pochi parametri** in `config.php` (il campo `$TB_HOST` e i `$TB_DEVICES`), senza nessun'altra modifica al codice.

ThingsBoard gestisce 5 dispositivi configurati nel sistema.

### Funzioni principali

- ricezione e storicizzazione dei dati in un database time-series;
- generazione e gestione degli allarmi tramite rule engine;
- esposizione di API REST per l'accesso ai dati dal backend PHP.

**Autenticazione verso ThingsBoard:**  
Il backend PHP si autentica con username e password tramite `POST /api/auth/login`. Il token JWT ricevuto viene conservato in sessione PHP e riutilizzato per un'ora prima di essere rinnovato automaticamente.

---

## Backend Applicativo (PHP)

Il backend è composto dai seguenti file con responsabilità distinte:

| File | Responsabilità |
|---|---|
| `api.php` | Proxy verso ThingsBoard: recupera telemetria (multi-cURL parallelo su tutti i dispositivi) e lista allarmi; espone un unico endpoint JSON al frontend |
| `config.php` | Credenziali ThingsBoard e ID dei 5 dispositivi |
| `db.php` | Connessione MySQL con rilevamento offline e caching dello stato di connessione in sessione |
| `auth.php` | Verifica sessione PHP, espone `isAdmin()`, `isOperator()`, `isViewer()`, `requireRole()` |
| `alarm_action.php` | Proxy per le azioni sugli allarmi (ack / clear) verso ThingsBoard; richiede ruolo ≥ operator |
| `impersonate.php` | Consente a un admin di navigare come un altro utente |
| `stop_impersonate.php` | Ripristina la sessione admin originale |
| `reset_password_admin.php` | Endpoint AJAX per il reset password di un utente da parte dell'admin |
| `force_reconnect.php` | Cancella il flag offline dalla sessione e forza un nuovo tentativo di connessione al DB |
| `logout.php` | Distrugge la sessione e reindirizza al login |

**Ottimizzazioni di `api.php`:**

- **Cache telemetria**: i dati vengono salvati in sessione PHP per 5 secondi; le richieste successive alla prima non ricontattano ThingsBoard.
- **Multi-cURL parallelo**: le richieste ai 5 dispositivi vengono eseguite in parallelo tramite `curl_multi_*`.
- **Rilevamento dati stale**: per ogni dispositivo viene calcolato l'intervallo dall'ultimo timestamp:
  - > 12 ore → flag `is_stale = true` (interfaccia mostra warning giallo);
  - > 24 ore → flag `is_very_stale = true` (interfaccia mostra allarme rosso).

**Configurazione database duale:**  
`db.php` mantiene in parallelo due righe di connessione commentate/attive, per consentire il passaggio immediato tra l'ambiente locale (rete scolastica, `192.168.60.144`) e quello remoto (hosting scolastico, `iisvio-sxtzwa62.db.tb-hosting.com`). Il cambio di ambiente richiede solo di invertire quale riga è commentata.

---

## Database

Il sistema utilizza **MySQL** con una singola tabella applicativa:

**`arnie_users`**

| Campo | Tipo | Note |
|---|---|---|
| `id` | INT AUTO_INCREMENT | chiave primaria |
| `nome` | VARCHAR(100) UNIQUE | username |
| `password_hash` | VARCHAR(255) | hash bcrypt |
| `ruolo` | ENUM('admin','operator','viewer') | default: viewer |
| `created_at` | TIMESTAMP | data creazione account |

La tabella è gestita interamente dall'applicazione PHP; ThingsBoard non ha visibilità su questi utenti.

---

## Modalità Offline

Se il database MySQL non è raggiungibile (timeout di connessione: 2 secondi), il sistema entra automaticamente in **modalità offline**:

- viene definita la costante `OFFLINE_MODE = true`, riconosciuta da tutti i file PHP;
- lo stato offline viene salvato in sessione con il timestamp di rilevamento;
- i nuovi tentativi di connessione vengono soppressi per **10 minuti** per evitare rallentamenti a ogni caricamento di pagina;
- nella navbar compare un badge "⚠ Offline" con un pulsante che richiama `force_reconnect.php` per forzare un tentativo immediato;
- se la connessione viene ristabilita, la sessione viene ripulita e viene mostrato un toast di notifica "Database raggiunto";
- il login in modalità offline avviene tramite **credenziali hardcoded** distinte per ruolo (`adminOffline`, `operatorOffline`, `viewerOffline`), con password fissa `offline1234`;
- le funzionalità che richiedono il DB (cambio password, gestione utenti) mostrano un messaggio di indisponibilità.

---

## Frontend / Web Application

### Tecnologie utilizzate

- HTML5, CSS3
- Bootstrap 5.3.2
- JavaScript (vanilla ES6+)
- **Chart.js 4.4.0** per la visualizzazione grafica
- **chartjs-plugin-zoom 2.0.1** + **HammerJS 2.0.8** per zoom e pan sui grafici (incluso pinch su mobile)
- **Lucide** per le icone
- **Inter** (Google Fonts) come font principale

### Pagine dell'applicazione

| File | Contenuto |
|---|---|
| `login.php` | Autenticazione; gestisce sia la modalità online che offline |
| `index.php` | Dashboard principale: statistiche globali, griglia arnie, storico allarmi, panoramica grafici, analisi avanzata |
| `arnie.php` | Dettaglio singola arnia: metriche, livello miele, stato sensori, storico allarmi, grafici storici |
| `allarmi.php` | Gestione allarmi attivi con filtri e modale di cambio stato |
| `archivio.php` | Allarmi risolti |
| `profile.php` | Profilo utente e cambio password |
| `utenti.php` | Gestione utenti (solo admin) |

### Navigazione — Sidebar

La navigazione è gestita da una **sidebar a comparsa** (hamburger menu), attiva su tutti i dispositivi. Componenti:

- overlay semitrasparente con chiusura al click o tasto ESC;
- voci di navigazione con evidenziazione del link attivo (basata su nome file e parametro `?id=` per le arnie);
- **status dot** per ciascuna arnia (verde/giallo/rosso/offline), aggiornati ogni 30 secondi tramite polling REST;
- switch "Dati demo" per la modalità mock;
- sezione admin ("Gestione Utenti") visibile solo agli admin;
- pulsante logout, sostituito dal pulsante "Esci da [utente]" durante l'impersonazione.

### Modalità Demo (Mock)

Tutte le pagine supportano una **modalità demo**, attivabile tramite lo switch in sidebar o nella toolbar delle pagine. Lo stato è persistito in `localStorage` con chiave `mockMode`. In modalità demo i dati reali non vengono richiesti: vengono usati i dati statici definiti in `dati.js` con variazioni casuali per i grafici.

---

## Grafici (Chart.js)

Tutti i grafici supportano zoom con rotella del mouse e pan orizzontale. Su mobile è supportato il pinch-to-zoom.

### Dashboard — tab Panoramica

| Grafico | Tipo | Contenuto |
|---|---|---|
| Andamento Temperature | Line (multi-dataset) | Una linea per arnia + linea media aggregata |
| Livelli Umidità | Area chart | Media aggregata di tutte le arnie |
| Produzione (peso totale) | Line chart | Somma aggregata del peso da tutte le arnie |
| Frequenza Sonora | Line chart | Media aggregata della frequenza di picco |

### Dashboard — tab Analisi Avanzata

| Grafico | Tipo | Contenuto |
|---|---|---|
| Correlazione Temp. Interna / Esterna | Scatter plot | Coppie di valori appaiati per timestamp; mostra R² |
| Velocità Variazione Temperatura | Bar chart (colorato) | Derivata discreta °C/h; barre ambra (riscaldamento) / blu (raffreddamento) |
| Rateo Variazione Peso Totale | Bar chart (colorato) | Derivata discreta kg/h; barre verdi (produzione) / rosse (perdita) |

### Dettaglio Arnia

| Grafico | Tipo | Contenuto |
|---|---|---|
| Temperatura Interna vs Esterna | Line (doppio dataset) | Serie temporale per l'intervallo selezionato |
| Umidità Interna | Area chart | Serie temporale |
| Variazione Peso | Bar chart | Serie temporale |
| Frequenza Picco (Hz) | Line chart | Serie temporale; asse Y suggerito 150–600 Hz |

**Selettore temporale:** 24 ore, 7 giorni, 1 mese, 1 anno.  
**Dati mancanti:** i grafici mostrano linee spezzate in presenza di buchi temporali; se nessun dato è disponibile per l'intervallo selezionato, il grafico viene sfocato e mostra un overlay "Nessun dato registrato in questo intervallo".

**Coefficiente R² Pearson:** calcolato lato client tra temperatura interna ed esterna, visualizzato come valore numerico nella card dedicata e nel grafico scatter della dashboard.

---

## Allarmi

### Flusso di gestione

Gli allarmi originano da ThingsBoard (rule engine). Il frontend li recupera tramite `api.php` con il campo `status` grezzo di ThingsBoard, che viene mappato in tre stati UI:

| Stato ThingsBoard | Stato UI |
|---|---|
| ACTIVE_UNACK | Da Gestire (system) |
| ACTIVE_ACK | Aperto (open) |
| CLEARED_UNACK / CLEARED_ACK | Risolto (closed) |

**Override locale:** lo stato effettivo di un allarme può essere sovrascritto dall'utente tramite `localStorage` (chiave `alarmStates`). Questo override ha priorità sullo stato ThingsBoard e persiste tra le sessioni nel browser.

**Azioni su ThingsBoard:** le azioni ack (→ ACTIVE_ACK) e clear (→ CLEARED_ACK) vengono inviate a ThingsBoard tramite `alarm_action.php`, che le delega alle API REST `/api/alarm/{id}/ack` e `/api/alarm/{id}/clear`. Solo gli utenti con ruolo operator o admin possono eseguire queste azioni.

**Pagina Allarmi (`allarmi.php`):** mostra gli allarmi attivi raggruppati per arnia, con filtri per stato, badge contatori, modale di cambio stato, e switch demo. Gli allarmi risolti non appaiono qui ma sono consultabili in `archivio.php`.

**Nota comportamentale:** aprire il modale di un allarme "Da Gestire" lo segna automaticamente come "Aperto". Chiudere il modale con X senza salvare produce lo stesso effetto.

---

## Gestione Utenti (admin)

La pagina `utenti.php` (accessibile solo agli admin, bloccata in modalità offline) consente:

- **Creazione account**: username, password (min. 8 caratteri, hash bcrypt), ruolo.
- **Cambio ruolo**: tramite select con submit immediato; non applicabile al proprio account.
- **Eliminazione utente**: con conferma; non applicabile al proprio account.
- **Reset password**: modale AJAX con validazione client-side (forza password, controllo coincidenza), invio a `reset_password_admin.php`; non applicabile al proprio account.
- **Impersonazione**: l'admin può navigare l'interfaccia come se fosse un altro utente, tramite `impersonate.php`. La sessione admin originale viene salvata; durante l'impersonazione compare un banner persistente con il nome dell'utente attivo e il pulsante "Torna a [admin]". La funzione è bloccata in modalità offline.

---

## Sicurezza e Gestione Accessi

### Autenticazione

- Sessioni PHP server-side; nessun token JWT lato client.
- Password memorizzate con `password_hash()` / `PASSWORD_BCRYPT`.
- In modalità offline: credenziali hardcoded separate da quelle reali.

### Ruoli

| Ruolo | Permessi |
|---|---|
| **viewer** | Lettura dati e grafici; nessuna modifica agli allarmi |
| **operator** | Tutto il viewer + gestione allarmi (ack, clear, cambio stato) |
| **admin** | Tutto l'operator + gestione utenti, reset password, impersonazione |

### Protezione lato server

- `auth.php` verifica la sessione su ogni pagina; reindirizza al login se assente.
- `requireRole()` blocca con HTTP 403 se il ruolo è insufficiente.
- I viewer che tentano azioni sugli allarmi via `alarm_action.php` ricevono HTTP 403.
- `alarm_action.php` accetta solo valori `action` ∈ {`ack`, `clear`}.

### Protezione dati ThingsBoard

- Le credenziali ThingsBoard risiedono esclusivamente in `config.php` lato server.
- Il token di sessione ThingsBoard non viene mai esposto al browser.
- `CURLOPT_SSL_VERIFYPEER` è disabilitato per ambienti di test interni; da riabilitare in produzione.

---

## Comunicazione e Protocolli

### Sensori → Gateway → ThingsBoard

- **Sensori → Gateway**: protocollo LoRa Custom (sviluppato dal gruppo hardware)
- **Gateway → ThingsBoard**: MQTT con TLS (protocollo principale), HTTP POST (alternativo)

### Frontend → Backend PHP → ThingsBoard

- Il frontend esegue richieste REST verso `api.php` (stesso dominio).
- **Non viene usato WebSocket**: l'aggiornamento dei dati avviene tramite **polling REST**:
  - dati arnie e status dot nella navbar: ogni 30 secondi;
  - allarmi: al caricamento pagina + pulsante "Aggiorna" manuale.
- `api.php` comunica con ThingsBoard tramite cURL (REST).

### Frequenza di aggiornamento e natura non real-time

Il sistema **non è real-time** per una scelta architetturale a livello fisico: i sensori inviano i dati a ThingsBoard **a blocchi ogni circa 8 ore** (salvo eventi critici). Questo significa che anche se il frontend interrogasse ThingsBoard ogni secondo, i valori visualizzati cambierebbero comunque solo alla ricezione di un nuovo blocco dati.

Il polling del frontend ogni 30 secondi serve quindi non a ottenere dati istantanei, ma a **recepire tempestivamente l'eventuale nuovo blocco** non appena arriva su ThingsBoard, senza richiedere un refresh manuale della pagina.

| Elemento | Frequenza lato frontend | Frequenza aggiornamento reale |
|---|---|---|
| Stato dot arnie nella sidebar | Ogni 30 secondi | ~ogni 8 ore (dipende dai sensori) |
| Dati arnie nella dashboard | Ogni 30 secondi | ~ogni 8 ore |
| Allarmi | Caricamento pagina / manuale | All'occorrenza (rule engine TB) |
| Indicatore "dati non aggiornati" | — | Calcolato dal timestamp dell'ultimo dato ricevuto |

---

## Notifiche

Le notifiche push (app mobile, email per allarmi critici) sono presenti nell'interfaccia nella pagina profilo con la scritta **"In sviluppo"** e non sono funzionanti. Tutta la sezione è coperta da un overlay visivo che indica l'indisponibilità della funzionalità.

---

## Scalabilità e Manutenibilità

- Il sistema gestisce attualmente **5 arnie**, configurate tramite array in `config.php`.
- La struttura PHP consente di aggiungere nuovi dispositivi modificando solo `config.php` e `dati.js`.
- Il frontend è modulare: i file JS sono separati per responsabilità (`alarm_state.js`, `thingsboard.js`, `allarmi.js`, `archivio.js`, `arnie.js`, `index.js`, `navbar.js`).
- Il CSS è organizzato per pagina (`style.css`, `navbar.css`, `arnie.css`, `allarmi.css`, `profile.css`, `login_signup.css`).
- Il cambio di ambiente database (locale/remoto) richiede la modifica di una sola riga in `db.php`.
