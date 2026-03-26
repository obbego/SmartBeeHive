# Analisi tecnica
Di seguito viene riportata l'analisi tecnica suddivisa per i vari componenti facenti parte del sistema dell'alveare. 

## Sistema sensoristico
Per la misura dei parametri all'interno dell'alveare si fa uso di un sistema sensoristico formato da:
- **Celle di carico**, in grado di misurare il peso dell'alveare per monitorare la quantità di miele prodotto all'interno delle arnie. Per la misura del peso vengono poste quattro di queste agli angoli dell'arnia, connesse a una scheda per l'interfacciamento con il microcontrollore.
- **DHT22**, un sensore di umidità e temperatura, specializzato per la sopportazione dei parametri atmosferici che si possono trovare all'esterno dell'arnia, come pioggia o alte o basse temperature. Questo sensore viene posto all'esterno dell'arnia per la cattura delle condizioni esterne.
- **KY-037**, un sensore microfonico per la misura dell'attività dell'alveare all'interno dell'arnia. Dal microcontrollore, all'utilizzo di questo sensore, vengono prodotti due parametri, ovvero l'intensità e la frequenza di picco del rumore prodotto all'interno dell'alveare, che ne permettono la misura dell'attività.

Nel sistema si trova inoltre un microcontrollore con lo scopo di misurare questi parametri. Questo è basato su *ESP32* con supporto per *LoRa* tramite chip *SX1262*.

## Gateway arnie
Il gateway è l'elemento che si occupa della ricezione delle informazioni dalle arnie stesse, per permettere un invio di queste verso il ricevitore tramite il protocollo *Niagara*, che successivamente si occuperà dell'invio a ThingsBoard per l'aggiunta delle misure al database.  
Il gateway ha lo scopo di attendere una richiesta dal ricevitore delle misurazioni, in quanto quest'ultimo, essendo connesso a Internet, è al corrente della data e dell'ora e ha l'obiettivo di richiedere tre misure giornaliere alle arnie.  
Il sistema sensoristico può effettuare più misure di quelle che vengono richieste dal ricevitore, effettuando solamente l'invio di questi dati sulla rete *LoRa* su richiesta da parte di quest'ultimo. Ciò avviene quando si deve fornire una lettura media delle misure attraverso l'intervallo delle otto ore fra una misura e l'altra, anziché una lettura istantanea.

## Ricevitore misurazioni
La ricezione delle misurazioni viene effettuata attraverso un software in esecuzione su un *Raspberry Pi* che implementa il protocollo *Niagara* per la comunicazione con le arnie.  
Sarà proprio il ricevitore delle misurazioni che si occuperà del timing, ovvero del conteggio del tempo trascorso fra una misura e l'altra, per permettere di inviare una richiesta al gateway a certi intervalli di tempo.  
Successivamente, alla ricezione delle misurazioni da parte delle arnie, viene stabilita una connessione HTTP con il server ThingsBoard per l'invio di questi dati e il salvataggio nel database.

### Dati necessari
Il ricevitore dovrà contenere al suo interno, per ogni arnia, il corrispondente token di accesso al server ThingsBoard. 

Infatti, per motivi riguardanti la MTU size del protocollo LoRa, l'identificativo che il ricevitore otterrà dalla comunicazione con le arnie sarà una stringa di caratteri di dimensione ridotta, che permette una comunicazione più agevole a differenza del token. 

Il file si troverà dentro alla cartella con il codice del ricevitore `devices.txt` dove all'interno saranno presenti i dati che rispetteranno la seguente formattazione:
```
DEVICE_1 TOKEN_1
DEVICE_2 TOKEN_2
...
```
Rispettare tale formattazione risulta fondamentale per il setup iniziale del ricevitore. 

Per archiviare i dati relativi ai dispositivi verrà utilizzata in ausilio la classe **DeviceInfo**, utilizzata anche nello scheduler e contenente, come attributi, l'identificativo e il corrispondente token di accesso per le richieste POST su ThingsBoard.

### Librerie richieste
Il ricevitore risulta di fatto il collegamento tra il sistema di comunicazione LoRa e l'archiviazione mediante ThingsBoard. Essendo inoltre realizzato in linguaggio C++, necessita l'installazione di librerie apposite che in altri linguaggi (come Python) potrebbero essere già disponibili. 

Di seguito vengono quindi riportate le librerie richieste con relativa funzione e motivo della scelta:
- ***Niagara*** libreria realizzata all'interno del progetto. Al suo interno contiene anche altri moduli da installare, come **lgpio** e **Radiolib**.
- ***curllib*** libreria utilizzata per fare richieste HTTP/HTTPS da codice mediante curl. Installazione semplice, ampiamemnte impiegata e base per molte altre librerie per la comunicazione HTTP/HTTPS. Dovendo eseguire il codice su un dispositivo Raspberry, conviene scegliere un package affidabile che risulti il più leggero possibile. 
- ***spdlog*** package per la gestione dei log. Dal momento che il codice rimarrà in esecuzione all'infinito, risulta fondamentale registarne le azioni in modo tale da essere consultate in caso di manutenzione/debug. **spdlog** è un package molto leggero (a differenza di altri) che supporta anche la "time rotation" per registrare solo l'ultimo periodo.

**N.B.** *Le indicazioni fornite riguardo l'adattabilità e agli aspetti delle librerie si riferiscono all'esercizio del codice all'interno di un Raspberry Pi con sistema operativo basato su kernel Linux. Non è garantito che tali considerazioni possano valere in ambienti diversi da quelli menzionati.*

### Funzioni codice
Il codice del ricevitore dovrà contenere le seguenti misurazioni:
- ***void initLogger()*** funzione per inizializzare il logger per registrare gli eventi.
- ***bool recoverDevices()*** funzione che recupera i dati relativi alle arnie per il salvataggio su ThingsBoard. Ritorna TRUE nel caso in cui l'operazione sia riuscita con successo.
- ***bool sendDataToThingsBoard()*** funzione che riceve i dati ed invia una richiesta POST al server ThingsBoard per il salvataggio delle telemetrie relative a quel dispositivo. La funzione ritorna TRUE se l'invio è riuscito è il server conferma la ricevuta delle telemetrie.

La ricezione via LoRa verrà effettuata all'interno della funzione **main** invocando le funzioni definite nella libreria di progetto **Niagara**. 
Il programma è monothread e monoprocesso, in quanto il protocollo LoRa e relativa libreria non prevede la ricezione simultanea dei messaggi. 
Ciò quindi costituisce una sorta di "imbuto", che però è da considerarsi accettabile visto la frequenza di invio delle telemetrie (3 volte al giorno). 


## Server ThingsBoard
Le parti da configurare del server ThingsBoard saranno quelle indicate di seguito.
Per ulteriori informazioni sul funzionamento e sull'analisi del server ThingsBoard visitare [questa pagina](./ThingsBoard.md).

### Dispositivi
Nella sezione `DEVICES` occore configurare **5 dispositivi**, uno per ogni arnia prevista inizialmente dai requisiti di progetto.
Ogni dispositivo dovrà avere i seguenti attributi impostati come `SHARED`:
- *temperature* in °C
- *weight* in kg
- *humidity* in %
- *noise-frequency* in Hz
- *noise-intensity* in dB

**N.B.** Ogni dispositivo avrà un ID e un token di accesso, che dovranno essere copiati e utilizzati dai dispositivi che si occuperanno di inviare le telemetrie, estrapolarle o mandare richieste RPC. 

### Asset
Nella sezione `ASSET` è necessario creare un asset comune che abbia una relazione di `CONTAINS` con tutti i dispositivi associati alle arnie. 
Tale soluzione contribuisce in fase di controllo del dato ad estrapolare anche i dati di altre arnie per effettuare meglio un confronto. 

### Inizializzazione dispositivi
Punto importante per la configurazione del server ThingsBoard è l'inizializzazione dei dispositivi. Per questo si è pensato di creare un profilo per i dispositivi corrispondenti alle arnie con attributi statici e campi calcolati che verranno definiti dal gruppo del frontend. I profili infatti contengono informazioni utili per l'output delle telemetrie nel frontend. 

L'aspetto che invece interessa il backend è la scelta di quali grandezze fisiche occorre accettare nelle telemetrie. Di default ThingsBoard accetta qualsiasi chiave inserita, ma per una migliore tolleranza agli errori potrebbe risultare conveniente specificare sin da subito quali grandezze fisiche ammettere. 
Infatti durante la fase di trasmissione, a causa della corruzione delle informazioni trasmesse o per un'errata configurazione dei dispositivi, potrebbe essere possibile avere delle telemetrie con nomi diversi da quelli attesi. 

**N.B.** *Tale funzionalità vuole essere solo un miglioramento che non costituisce una necessità. Il sistema funzionerà ugualmente anche senza sviluppare questo aspetto, semplicemente non definendo la lista di telemetrie da ammettere*.

Per gestire solo alcune grandezze fisiche come telemetrie, occorre sviluppare i seguenti punti:

#### Script di inizializzazione attributi
Questo script avviato una volta installato ThingsBoard si impegna a popolare i dispositivi inseriti nella piattaforma con un attributo condiviso contenente la stringa in formato JSON di tutte le grandezze fisiche accettate per la telemetria.

Tale script leggerà le telemetrie accettate da un file `allowed_keys.json` e deve rispettare il seguente formato:
```json
[
    {
        "devices":["device1", "device2"],
        "keys":["humidity", "temperature", "weight", "noise_intensity", "noise_frequency"]
    }
]
```
Come è possibile notare il file può gestire anche più liste di telemetrie diverse associate a dispositivi diversi. 

#### Rule node di controllo
All'interno della rule chain che sarà amputata al controllo della telemetria in ingresso che dovrà controllare la lista di telemetrie valide (se presente) e, se in tal caso non c'è corrispondenza, generare un allarme senza salvare la telemetria.


### Rule chains
Nel server Thingsboard, per rispondere alle esigenze individuate nell'analisi funzionale inerente al server, occorre creare / modificare i seguenti nodi. 

#### Root rule chain
Rule chain di base che è presente in qualsiasi server ThingsBoard. Quest'ultima si attiva qualsiasi richiesta arrivi verso il server. 
In questa sezione occorre applicare le seguenti modifiche:
1. **DEVICE PROFILE NODE** dopo questo nodo, se il dispositivo non è stato trovato, occorre lanciare un allarme di severità `CRITICAL` chiamato `DeviceNotFound` per segnalare che il dispositivo che ha tentato l'accesso non è stato trovato.
2. **POST TELEMETRY** in questo ramo dopo il `Message Type Switch` bisogna agganciare il nodo per il controllo della validità del dato. Se tale nodo restituirà esito positivo, il dato verrà salvato nel database con l'apposito nodo. 
3. **RPC REQUEST FROM DEVICE** nel ramo della richiesta RPC da dispositivo, sempre dopo il `Message Type Switch`, si deve fare un controllo del testo del messaggio, che indicherà se attivare il controllo del peso dell'arnia, se effettuare un controllo del dispositivo o nessuna delle due. 

#### Check weight beehive
Il seguente nodo si attiva dopo che viene determinata la natura di una richiesta RPC da un dispositivo.
La rule chain ha il compito di verificare se il peso è stabile nel tempo e indicare quindi se è possibile raccogliere il miele o meno:
1. **Ottenimento telemetria** ottenere le recenti telemetrie del dispositivo sul suo peso (indicativamente gli ultimi cinque giorni). In caso in cui ci sia un errore in questa fase va segnalato con gravità `Minor`.
2. **Calcolo variazione peso** questo nodo calcola la variazione del peso utilizzando la regressione lineare. Se la variazione risulta al di sotto di una certa soglia, verrà ritornato esito positivo. Il risultato dovrà comunque essere aggiunto al messaggio di partenza. 
3. **Switch condizione di peso** il messaggio aggiornato viene letto da uno switch che suddivide la rule chain in più casi. 
4. **Allarmi** in caso di esito positivo viene attivato l'allarme `HoneyReady`, in caso contrario viene disattivato se presente in precedenza.
5. **Response RPC** in qualsiasi caso verrà effettuata una risposta RPC contenente il messaggio aggiornato, per la corretta terminazione della richiesta effettuata. 

#### Check timeseries average
Questa rule chain azionata tramite apposita richiesta RPC ha lo scopo di calcolare per ogni dispositivo la media delle telemetrie fatte nell'ultimo periodo (indicativamente l'ultima settimana).
La rule chain sarà quindi composta da:
1. **Ottenimento telemetrie** dal dispositivo si ricavano tutte le telemetrie appartenenti all'intervallo scelto. 
2. **Valutazione telemetrie** se l'ultima telemetria per quella grandezza supera un certo intervallo di tempo (un giorno) viene segnalato un errore su quel dispositivo. 
3. **Calcolo media** viene calcolata la media per ogni grandezza trovata. 
4. **Salvataggio attributi** i dati vengono salvati su quel dispositivo.
 
#### Check telemetry validity
Questa rule chain viene attivata ogni qualvolta si desideri salvare una telemetria nel server ThingsBoard. 
Tale rule chain verifica la validità del dato con una logica che tenta di ridurre al minimo il margine di errore nella misurazione:
1. **Ottenimento medie misurazioni** dall'asset che contiene i dispositivi viene ricavata la media di tutte la grandezze disponibili.  
2. **Controllo inserimento dato** se il dato era già stato precedentemente inserito, verrà utilizzato nella valutazione della telemetria appena inserita. 
3. **Confronto con dato in input** se il dato inserito è il primo, questo verrà salvato nel database. Se invece il dato era stato già precedentemente salvato, esso verrà sostituito con quello attuale solo se si avvicina maggiormente alla media delle telemetrie di tutti i dispositivi in quell'orario. Questo viene fatto perchè ci potrebbero essere errori nella trasmissione del dato e questo potrebbe arrivare corrotto e quindi errato.
Il risultato viene salvato all'interno del messaggio con un apposito flag. 
4. **Aggiornamento messaggio** vengono eliminate le misurazioni considerate non valide e il messaggio verrà ritornato nella ***Root rule chain*** dove sarà presente il nodo di salvataggio nel database.

#### Check device status
Il dispositivo necessita in oltre controlli per verificare se il suo stato (ossia quello dei sensori) è ottimale, o ci sono dei dati che sono molto diversi rispetto alla media. 
La rule chain si occupa quindi di verificare tutte le grandezze fisiche misurate dal dispositivo e verificare su tutte se sono in linea con quelle degli altri dispositivi, a seguito di una richiesta RPC da un dispositivo.
La rule chain segue quindi i seguenti passaggi:
1. **Ottenimento grandezze** vengono estrapolate tutte le grandezze fisiche che il dispositivo misura e vengono aggiunte al messaggio.
2. **Ottenimento telemetrie** vengono estrapolati le telemetrie di tutte le grandezze fisiche di tutte le arnie nell'ultimo periodo (indicativamente l'ultima settimana). Queste verranno aggiunte al messaggio. 
3. **Calcolo medie e verifica** viene calcolata per ogni grandezza separatamente la media del dispositivo interessato e quella degli altri. Viene quindi verificato se le due medie differiscono oltre una certa percentuale di soglia. Tale risultato viene salvato per ogni grandezza attraverso un flag all'interno del messaggio. 
4. **Switch risultati** lo switch legge quindi i risultati della fase di calcolo e suddivide la rule chain in vari casi, tenendo conto che ci potrebbe essere anche più di una grandezza fisica fuori dalla media.
5. **Allarmi** per ogni segnalazione va generato un allarme. Sarà presente un allarme personalizzato per grandezze prospettate più uno per grandezze aggiuntive configurabili in futuro. In caso in cui l'esito sia negativo, l'allarme stesso va cancellato.
6. **Response RPC** in qualsiasi caso ritornare un messaggio con risposta RPC. 
7. **Salvataggio asset** la rule chain esegue il calcolo di una media generale tra il dispositivo corrente e gli altri dispositivi da salvare come attributo nell'asset contenitore dei dispositivi.

### Allarmi
La seguente sezione indica i possibili allarmi che potrebbero essere generati dal server ThingsBoard a seguito di richieste HTTP per invio telemetrie o RPC sempre controllate mediante Rule Chains.

| Nome allarme | Tipologia | Descrizione |
|------------|------------|------------|
| `DeviceNotFound` | Critical | Il dispositivo non viene trovato e la fase di autenticazione viene interrotta. |
| `DeviceDifferentTemperature` | Major | Il dispositivo ha una temperatura media molto diversa rispetto a quella degli altri dispositivi. |
| `DeviceDifferentHumidity` | Major | Il dispositivo ha un'umidità media molto diversa rispetto a quella degli altri dispositivi. |
| `DeviceDifferentWeight` | Major | Il dispositivo ha un peso medio molto diverso rispetto a quella degli altri dispositivi. |
| `DeviceDifferentNoiseIntensity` | Major | Il dispositivo ha un'intesità di rumore media molto diversa rispetto a quella degli altri dispositivi. |
| `DeviceDifferentNoiseFrequency` | Major | Il dispositivo ha una frequenza di rumore media molto diversa rispetto a quella degli altri dispositivi. |
| `ChangeOriginatorToAsset` | Minor | Il cambio di originatore del messaggio da dispositivo ad asset non è riuscito. |
| `ErrorDeviceTimeseries` | Minor | L'arricchimento del messaggio con le telemetrie del dispositivo non è riuscito.|
| `DeviceOldTemperature` | Major | L'ultima temperatura registrata supera un certo intervallo di tempo. |
| `DeviceOldHumidity` | Major | L'ultima umidità registrata supera un certo intervallo di tempo. |
| `DeviceOldWeight` | Major | L'ultimo peso registrato supera un certo intervallo di tempo. |
| `DeviceOldNoiseFrequency` | Major | L'ultima frequenza di rumore registrata supera un certo intervallo di tempo. |
| `DeviceOldNoiseIntensity` | Major | L'ultima intensità di rumore registrata supera un certo intervallo di tempo. |
| `ErrorTimeseriesWeightDevice` | Minor | L'arricchimento con le telemetrie di peso del dispositivo non è riuscito. |
| `HoneyReady` | Warning | Avviso che è possibile raccogliere il miele |
| `FailedAssetAttributes` | Minor | Recupero degli attributi dell'asset non riuscito. |
| `TelemetryInvalidKey` (Opzionale) | Major | Telemetria inserita non ha il nome della chiave corrispondente a quelli accettati dal server riguardo il dispositivo. |

### Database
Il database ThingsBoard dispone anche della possibilità di aggiungere tabelle aggiuntive utilizzando il motore Cassandra. 
La tabella deve essere già configurata nei file di configurazione del server e vi è un nodo specifico che consente di agire su questa funzionalità. 

Nello specifico la tabella si chiamerà `log-telemetries-action` e conterrà:
- **PreviousTelemetry** telemetria già inserita, se presente
- **Timestamp** timestamp della telemetria
- **CurrentTelemetry** telemetria ripetuta con medesimo timestamp (potrebbe variare) 
- **DateTimeAction** data e ora in cui è stato effettuato il controllo
- **Action** azione effettuata dal controllo (`insert` se il dato è appena stato inserito, `replace` se il nuovo dato rimpiazza quello precedente, `dropped` se il nuovo dato è stato scartato)
- **DeviceName** nome del device dove l'azione è stata performata 

**N.B.** Tale proposta viene per il momento non attuata in quanto l'utilizzo di un sistema ibrido con Cassandra salverebbe anche le telemetrie all'interno di questa tipologia di database. Per non modificare troppo la configurazione di Thingsboard e in assenza di una documentazione appropriata, si pianifica di trovare una soluzione o scartare la funzionalità di salvataggio, in quanto parte marginale e non necessaria per il funzionamento del sistema.

## Scheduler
Dal momento che la versione di ThingsBoard utilizzata non consente di eseguire controlli periodici indipendentemente dai dati inseriti, occorre attivare uno scheduler che invii ad orari prestabiliti richieste RPC per attivare i suddetti controlli.

### Classi e oggetti
Per il corretto funzionamento delle richieste occorre creare una classe **DeviceInfo** che al suo interno contenga i seguenti dati:
- *commonName* nome dato alla scheda di sensori collegata all'ania
- *accessToken* access token del dispositivo per l'accesso a ThingsBoard
- *deviceId* id del device utile per alcune richieste

**N.B.** L'ID del dispositivo potrebbe essere non necessaria per le richieste da effettuare. 
 
### Variabili
In particolare il codice per il suo funzionamento dovrà essere composto dai seguenti array/variabili/costanti:
- `timesCheckWeightBeehive[]` array di ore in cui eseguire i controlli per il peso dell'arnia.
- `timesCheckDeviceStatus[]` array di ore in cui eseguire i controlli sui sensori del dispositivo. 
- `lastTimeCheckWeightBeehive`ultima ora in cui è stata inviata la richiesta di controllo del peso dell'alveare. 
- `lastTimeCheckDeviceStatus` ultima ora in cui è stata inviata la richiesta per il controllo dello stato del dispositivo. 
- `THINGSBOARD_TOKENKJWT` token per l'accesso su ThingsBoard.
- `THINGSBOARD_HOSTNAME` nome del dispositivo (indirizzo IP e numero di porta) per effettuare le richieste- 

**N.B.** Il token JWT potrebbe non essere necessario in fase di richiesta RPC. 

### Funzioni
Il programma dovrà implementare principalmente le seguenti funzioni:
- `scheduler_checkWeightBeehive()` funzione che invia la richiesta di controllare il controllo del peso. Sarebbe ottimale ritornare l'esito dell'operazione.
- `scheduler_checkDeviceStatus()` funzione che invia la richiesta di controllare lo stato del dispositivo. Sarebbe ottimale ritornare l'esito dell'operazione.

### Note tecniche
Tale scheduler potrebbe essere implementato nel ricevitore dei dati dall'alveare, visto che è predisposto per il collegamento al server ThingsBoard. 

Nel momento stabilito verrà fatta una richiesta RPC per ogni dispositivo, con un leggero ritardo tra una e l'altra. 

### Gestione errori
Lo scheduler del PC rappresenta un elemento potenzialmente rischioso all'interno del sistema, in quanto non è incluso nativamente in ThingsBoard ma è un'aggiunta esterna e personalizzata. Ne consegue che potrebbero esserci problemi nell'esecuzione delle richieste, e nell'estrapolazione dell'output richiesto. 

Proprio per questo lo scheduler disporrà di un log per la fase di debug inziale e per la durata del suo funzionamento, in modo tale da rilevare le ultime operazioni e i principali errori.

Inoltre **ogni richiesta potrà essere ripetuta al massimo 3 volte**, in modo da scongiurare problemi dovuti ad una possibile congestione della rete o di prima richiesta errata.
