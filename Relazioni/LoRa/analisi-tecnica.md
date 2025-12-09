# Analisi tecnica
Di seguito viene riportata l'analisi tecnica suddivisa per i vari componenti facenti parte del sistema dell'alveare. 

## Sistema sensoristico


## Gateway arnie


## Ricevitore misurazioni


## Server ThingsBoard
Le parti da configurare del server ThingsBoard saranno quelle indicate di seguito.
Per ulteriori informazioni sul funzionamento e sull'analisi del server ThingsBoard visitare [questa pagina](./ThingsBoard.md).

### Dispositivi
Nella sezione `DEVICES` occore configurare **5 dispositivi**, uno per ogni arnia prevista inizialmente dai requisiti di progetto.
Ogni dispositivo dovrà avere i seguenti attributi impostati come `SHARED`:
- *temperature* in °C
- *weight* in kg
- *humidity* in %
- *noise* in Hz

**N.B.** Ogni dispositivo avrà un ID e un token di accesso, che dovranno essere copiati e utilizzati dai dispositivi che si occuperanno di inviare le telemetrie, estrapolarle o mandare richieste RPC. 

### Asset
Nella sezione `ASSET` è necessario creare un asset comune che abbia una relazione di `CONTAINS` con tutti i dispositivi associati alle arnie. 
Tale soluzione contribuisce in fase di controllo del dato ad estrapolare anche i dati di altre arnie per effettuare meglio un confronto. 

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
| `DeviceDifferentNoise` | Major | Il dispositivo ha un rumore medio molto diverso rispetto a quella degli altri dispositivi. |
| `ChangeOriginatorToAsset` | Minor | Il cambio di originatore del messaggio da dispositivo ad asset non è riuscito. |
| `ErrorDeviceTimeseries` | Minor | L'arricchimento del messaggio con le telemetrie del dispositivo non è riuscito.|
| `DeviceOldTemperature` | Major | L'ultima temperatura registrata supera un certo intervallo di tempo. |
| `DeviceOldHumidity` | Major | L'ultima umidità registrata supera un certo intervallo di tempo. |
| `DeviceOldWeight` | Major | L'ultimo peso registrato supera un certo intervallo di tempo. |
| `DeviceOldNoise` | Major | L'ultimo rumore registrato supera un certo intervallo di tempo. |
| `ErrorTimeseriesWeightDevice` | Minor | L'arricchimento con le telemetrie di peso del dispositivo non è riuscito. |
| `HoneyReady` | Warning | Avviso che è possibile raccogliere il miele |
| `FailedAssetAttributes` | Minor | Recupero degli attributi dell'asset non riuscito. |

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