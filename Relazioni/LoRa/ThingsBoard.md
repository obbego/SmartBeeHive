# ThingsBoard
## Cos'è
ThingsBoard è la piattaforma scelta per la raccolta dei dati delle varie arnie. Il software dovrà essere configurato sul server prescelto per poter poi utilizzare le informazioni salvate.
Il server configurato potrà disporre di un'interfaccia predefinita per mostrare i dati raccolti, scegliendo quali informazioni esporre e come visualizzarle.

**N.B.** *Poiché rientra tra i progetti della scuola la realizzazione dell'interfaccia grafica per i dati dell'alveare, questa relazione tratterà soltanto l'aspetto tecnico del server, spiegando cosa è utile per interagire con esso e per inserire/estrarre i dati.*


## Architettura della piattaforma
![Architettura ThingsBoard](img/thingsboard-structure.jpg)

L'architettura di ThingsBoard è costituita da vari elementi; di seguito riportiamo i principali.

### Core
Il ***Core*** di ThingsBoard è, come dice il nome, la parte centrale della piattaforma. Esso serve a:
- Gestire le chiamate API da altri dispositivi
- Gestire attributi e dati di telemetria
- Processare i messaggi provenienti dal [*Rule engine*](#rule-engine)
- Monitorare lo stato di connettività (attivo/disconnesso)

### Rule engine
Il ***Rule engine*** è la componente di ThingsBoard che stabilisce le regole, le condizioni e le azioni da compiere per uno specifico evento o dato. È possibile eseguire confronti, decidere se salvare un dato o generare allarmi.

Il rule engine è costituito dai seguenti elementi:
- **Message**: qualsiasi evento. Può essere un dato, una richiesta, una chiamata API, ecc. Esso è composto dal corpo del messaggio (msg), dai dati correlati al messaggio (metadata) e dai tipi di messaggio [msgType](https://thingsboard.io/docs/user-guide/rule-engine-2-0/overview/#predefined-message-types).
- **Rule node**: funzione che agisce su un messaggio e può compiere trasformazioni o elaborazioni del dato.
- **Rule chain**: connessione tra nodi che permette di trasferire l'output di un nodo come input di un altro.

![*rule engine*](img/rule-engine.png)
*Esempio di rule engine configurabile in ThingsBoard*

#### Linguaggi per i Rule Node
I linguaggi utilizzati per i *Rule Node* sono:
- **TBEL**: linguaggio di ThingsBoard con struttura simile a Java
- **JavaScript**: linguaggio ampiamente conosciuto e compatibile per i nodi di ThingsBoard

### Database
Tutti i dati inviati a ThingsBoard vengono salvati nei database, che contengono:
- **entities**: dispositivi, clienti, dashboard, ecc.; rappresentano la configurazione dell'ambiente ThingsBoard.
- **telemetries**: rilevazioni dei sensori, timestamp, statistiche ed eventi.

Tali informazioni possono essere salvate tramite due tipi di database:
- **SQL** (impostazione predefinita). Si consiglia PostgreSQL, in quanto ampiamente supportato da ThingsBoard.
- **Ibridi**: salvando le *entities* in PostgreSQL e i dati telemetrici usando Cassandra o TimescaleDB.


## Entità e relazioni
### Tipologie di entità
Come si è visto in precedenza, ThingsBoard contiene diverse entità utili al funzionamento della piattaforma. Sono:
- **Tenant**: profilo che identifica un singolo o un'organizzazione che possiede dispositivi e asset. A un tenant possono essere associati clienti che visualizzano le risorse.
- **Customer**: profilo che consente l'accesso in sola visualizzazione ai dispositivi già configurati dal tenant. Un customer può avere più utenti e più dispositivi e asset da visualizzare.
- **User**: profilo con accesso alle dashboard e alla gestione delle entità.
- **Dispositivo**: entità che produce dati di telemetria e gestisce comandi RPC.
- **Asset**: entità astratte che possono essere collegate a dispositivi o altri asset (es. veicolo, campo).
- **Entity View**: configurazione per mostrare solo una parte di dispositivi e asset al profilo customer.
- **Alarms**: eventi che notificano problemi o informazioni relative a dispositivi, asset o entità.
- **Dashboard**: interfaccia per la visualizzazione dei dispositivi IoT e per la costruzione dell'interfaccia grafica.

### Tipologie di relazioni
Le relazioni collegano dispositivi, asset e altre entità presenti all'interno dello stesso tenant.
I tipi di relazione possono essere, ad esempio:
- *Contiene*
- *Gestisce*
- *ecc.*

Queste relazioni vengono poi salvate nella parte del database ThingsBoard dedicata alle entità.

![Entità/Relazioni](img/entita-relazioni.svg)
*Esempio di schema entità/relazione utilizzato in ThingsBoard*

## Comunicazione con ThingsBoard
### Protocolli supportati per il salvataggio
Per l'invio dei dati al server ThingsBoard è possibile utilizzare diversi protocolli e API, tra cui:
- **HTTP**
- **MQTT**
- **CoAP**
- **LwM2M**

### Esempio con HTTP
Di seguito sono riportate alcune istruzioni per comunicare con il server ThingsBoard per il salvataggio e la richiesta dei dati.

#### URL del server
L'URL da utilizzare per la comunicazione con il server ThingsBoard è:
```http
http(s)://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/telemetry
```
dove:
- *$THINGSBOARD_HOST_NAME*: hostname o indirizzo IP del server.
- *$ACCESS_TOKEN*: token del dispositivo (disponibile una volta creato il dispositivo in ThingsBoard).

La connessione al server può restituire i seguenti errori comuni:
- **400 Bad Request**: errore nella richiesta (formattazione, endpoint errato, ecc.)
- **401 Unauthorized**: token non valido o mancante
- **404 Not Found**: risorsa non trovata

#### POST via HTTP
Ecco come inviare dati al server ThingsBoard tramite HTTP POST.

Un esempio di comando curl per inviare i dati:
```http
curl -v -X POST -d @telemetry-data-with-ts.json https://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/telemetry --header "Content-Type:application/json"
```
Il parametro `@telemetry-data-with-ts.json` indica il file JSON che contiene i dati da inviare.

I dati possono essere inviati con un timestamp già specificato (scelta consigliata per le misurazioni):
```json
{
  "ts": 1451649600512,
  "values": {
    "stringKey": "value1",
    "booleanKey": true,
    "doubleKey": 42.0,
    "longKey": 73,
    "jsonKey": {
      "someNumber": 42,
      "someArray": [1, 2, 3],
      "someNestedObject": {
        "key": "value"
      }
    }
  }
}
```
**N.B.** Per il timestamp è consigliato inserire anche i millisecondi, visti gli esempi forniti dalla documentazione.

oppure è possibile omettere il timestamp e lasciare che il server lo assegni automaticamente:
```json
{
  "stringKey": "value1",
  "booleanKey": true,
  "doubleKey": 42.0,
  "longKey": 73,
  "jsonKey": {
    "someNumber": 42,
    "someArray": [1,2,3],
    "someNestedObject": {"key": "value"}
  }
}
```

**N.B.** Una volta che viene effattuata una richiesta di POST via HTTP, il messaggio viene processato da Thingsboard, dove il timestamp specificato viene inserito all'interno dei metadata mentre nel corpo del messaggio rimangono solo le chiavi e i valori per la telemetria.

**N.B.** *È possibile inviare i dati in formato JSON con i comandi di POST, ma risulta più comodo utilizzare un file JSON per dati complessi con molte chiavi o oggetti. La soluzione adottata con il file JSON è ottimale per le necessità del progetto.*

#### GET via HTTP
Per ottenere una serie di dati telemetria occorre digitare la seguente richiesta HTTP sfruttando le chiamate API di ThingsBoard:
```http
curl -X GET "http://192.168.60.150:8080/api/plugins/telemetry/DEVICE/<deviceId>/values/timeseries?keys=temperature&startTs=1697000000000&endTs=1697600000000&limit=10" -H "X-Authorization: Bearer <JWT_TOKEN>"
```
dove:
- *deviceId* è il nome del dispositivo, ricavabile dal server ThingsBoard
- *startTs* timestamp con il momento iniziale da cui ottenere la telemetria
- *endTs* timestamp con il momento finale con il quale terminare la ricezione della telemetria
- *JWT_TOKEN* token web per l'utente ottenibile sempre dal server ThingsBoard
- *limit* numero di telemetrie massime da ottenere

Per utilizzare solo l'intervallo o solo il numero di telemetrie è possibile omettere l'indicazione dei timestamp o del limite. 

L'output tipico di una richiesta simile è il seguente:
```json
{"key":{"timestamp":1451649600512,"value":"value"}}
```

Per ottenere invece informazioni relative agli allarmi di un gruppo di dispositivi è possibile utilizzare la seguente richiesta: 
```http
curl -X GET "https://<tuo-dominio>/api/alarms?startTime=1729400000000&endTime=1729600000000&pageSize=50&page=0&sortOrder=DESC" -H "X-Authorization: Bearer <JWT_TOKEN>"
```
dove:
- *startTime* e *endTime* indicano l'intervallo in millisecondi
- *pageSize* quante righe per pagina
- *page* numero della pagina da cui iniziare
- *sortOrder* di tipo ASC o DESC
- *sortProperty* campo su cui ordinare

Il risultato atteso è il seguente:
```json
{
  "data": [
    {
      "id": {"id": "e8d96f30-6fd1-11ee-a3b2-2b3f5b743d8e"},
      "type": "HighTemperature",
      "severity": "CRITICAL",
      "status": "ACTIVE_UNACK",
      "originator": {
        "entityType": "DEVICE",
        "id": "e50d2f20-5a6a-11ee-b3f1-2f3c3d4479cc"
      },
      "createdTime": 1729433823000,
      "details": {
        "temperature": 82.5
      }
    }
  ],
  "totalPages": 1,
  "totalElements": 1,
  "hasNext": false
}
```

**N.B.** Tali richieste sono degli esempi di come inviare o ricevere dati collegandosi al server ThingsBoard. Ciò non esclude che possano essere presenti altre informazioni da inserire nelle richieste per inviare dati o filtrarli secondo altre specifiche. Tali informazioni è possibile trovarle consultando la documentazione a fine pagina.

#### Richiesta RPC
Una richiesta RPC (Remote Procedure Call) è chiamata che viene effettuata da un dispositivo client per eseguire istruzioni su un dispositivo remoto. 
In questo caso ThingsBoard supporta le chiamate RPC da dispositivi per poter eseguire istruzioni definibili nelle rule chain.

Tale richiesta può essere sempre fatta in HTTP ed è la seguente:
```http
curl -v -X POST -d @telemetry-data-with-ts.json https://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/rpc --header "Content-Type:application/json"
```
I dati da inserire nel file json devono attenersi a questo formato:
```json
{
    "method":"METHOD_NAME",
    "params":{
      "param1":"PARAM"
    }
}
```

Ogni richiesta RPC in ThingsBoard è sincrona, quindi il server dovrà inviare una risposta per il corretto completamento della procedura.


## Funzionamento per l'alveare
### Utilizzo del Rule Engine
Stando alla documentazione precedente è possibile salvare telemetrie per ogni dispositivo utilizzando il sever ThingsBoard.
Tuttavia, oltre a raccogliere semplicemente i dati, il sistema dell'alveare dovrebbe poter:
- segnalare lo stato dell'alveare (come quando è ora di raccogliere il miele)
- segnalare possibili anomalia del sistema di sensori
- controllare la validità del dato trasmesso

Per questo occorrerà interagire con le Rule Engine e impostare delle Rule chains che possano controllare queste condizioni e segnalarle. 

L'obiettivo è quello di mostrare questi messaggi come allarmi in modo che possano essere letti e mostrati anche nel frontend previsto per il progetto dell'alveare. 

### Asset comune
Tutti i dispositivi collegati alle arnie dovranno essere raggruppati in un asset comune. 
Potrebbe essere utile infatti poter confrontare dati tra arnie diverse e notare la presenza di possibili anomalie.

### Rule chains da creare
Per far sì che il sistema sia completo e possa segnalare informazioni o problemi del sistema occorre che siano implementate le seguenti Rule Chain:
- **controllo registrazione dato** se il dispositivo indicato non è stato trovato nel server occorre inserire un allarme di errore.
- **controllo dato** una volta passato il controllo del dispositivo occorre che il dato venga controllato, ossia che la grandezza da misurare sia già stata registrata e che il dato non sia fuori dal range previsto, possibile segnale di un'anomalia dei sensori o di un errore nella trasmissione del dato. In base a quello sarà possibile scegliere se o meno archiviare la telemetria o sostituirla con una già presente. In quel caso è possibile segnalare la cosa con un semplice allarme di rilevanza minore.
- **controllo dispositivo** deve essere controllato che il dispositivo abbia effettivamente ricevuto misurazioni nell'ultimo intervallo di tempo e che gli ultimi dati raccolti non sforino con la media dei dispositivi vicini. Per questo tipo di dato occorrerebbe inserire un warning. 
- **controllo alveare** per il controllo dell'alveare è possibile inserire un allarme che indica alcune informazioni relative all'alveare. Ad esempio con un warning si potrebbe indicare il momento in cui raccogliere il miele.

### Limitazioni nell'utilizzo
Durante alcune prove nell'utilizzo di una versione demo del server Thingsboard si sono riscontrati i seguenti problemi, equiparabili alla versione gratuita di cui l'alveare è dotato. 

1. **SCRIPT PERIODICI** non è possibile implementare in alcun modo una rule chain che venga eseguita periodicamente. Per farlo occorre utilizzare una RPC al server Thingsboard lanciata da uno scheduler in attività.
2. **RILEVAZIONE TELEMETRIE** non è possibile ricavare telemetrie multiple da un dispositivo in relazione con quello che sta eseguendo la rule chain, ma solo l'ultima registrata. Per fare una valutazione su più telemetrie occorre effettuare una media delle misurazioni. 
3. **RILEVAZIONE ATTRIBUTI** se ci sono attributi con lo stesso nome su più dispositivi, viene solo recuperato il primo trovato. Questo è un lato negativo nel caso in cui si debbano confrontare le medie della stessa grandezza di più dispositivi.


## Link utili
- [Architettura ThingsBoard](https://thingsboard.io/docs/reference/)
- [Entità e relazioni](https://thingsboard.io/docs/user-guide/entities-and-relations/)
- [Comunicazione con ThingsBoard](https://thingsboard.io/docs/reference/protocols/)
- [HTTP API](https://thingsboard.io/docs/reference/http-api/)
- [REST API](https://thingsboard.io/docs/user-guide/telemetry/#data-query-rest-api)
- [Custom table database](https://thingsboard.io/docs/user-guide/rule-engine-2-0/nodes/action/save-to-custom-table/)