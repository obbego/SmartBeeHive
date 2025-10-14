# ThingsBoard
## Cos'è
ThingsBoard è la piattaforma scelta per la raccolta dati delle varie arnie. Tale software dovrà essere configurato sul server prescelto per poter poi usufruire delle informazioni salvate
Il server configurato potrà disporre anche di un'interfaccia predefinita per mostrare i dati raccolti, selezionando quali e come esporli. 

**N.B.** *Dal momento che rientra tra i progetti della scuola realizzare l'interfaccia grafica per i dati dell'alveare, questa relazione affronterà solo il punto di vista tecnico del server, approfondendo cosa è utile per interagire con esso e inserire/estrapolare dati.*


## Architettura della piattaforma
![Architettura ThingsBoard](img/thingsboard-structure.jpg)

L'architettura di ThingsBoard è costituita da vari elementi, di seguito riportiamo i principali. 

### Core
Il ***Core*** di ThingsBoard è come dice, il nome, la parte centrale della piattaforma. Essa serve a:
- Gestire le chiamate API da altri dispositivi
- Agire su attributi e dati di telemetria
- Processare messaggi provenienti da [*Rule engine*](#rule-engine)
- Monitorare lo stato di connettività (attivo/disattivo)

### Rule engine
Il ***Rule engine*** è una parte di ThingsBoard che stabilisce le regole, le condizioni e le azioni da compiere per uno specifico dato. È possibile eseguire confronti, stabilire se salvare o meno il dato o lanciare degli allarmi.

Il rule engine è costituito dai seguenti elementi:
- **Message** è qualsiasi evento. Può essere un dato come una richiesta, una chiamata API ecc.
- **Rule node** funzione che agisce su un messaggio, quindi può compiere trasformazioni, lavorazioni del dato ecc.
- **Rule chain** connessione tra nodi che permette di trasferire l'output di un nodo come input di un altro. 

![*rule engine*](img/rule-engine.png)
*Esempio di rule engine configutarabile in ThingsBoard*

#### Linguaggio per Rule Node
Il linguaggio utilizzato per il *Rule Node* è:
- **TBEL** linguaggio di ThingsBoard con struttura java-like
- **Javascript** linguaggio java-like maggiormente conosciuto e compatibile per i nodi di ThingsBoard

#### Tipi predefiniti di messaggi
Esistono dei tipi predefiniti di messaggi intercettabili configurati in ThingsBoard raggiungibili tramite [questo link](https://thingsboard.io/docs/user-guide/rule-engine-2-0/overview/#predefined-message-types).

### Database
Tutti i dati inviati su ThingsBoard vengono salvati nei database, che contengono:
- **entities** come dispositivi, clienti, dashboard, ecc. Contengono quindi la parte di configurazione dell'ambiente ThinhsBoard. 
- **telemetries** rilevazioni sensori, tempi, statistiche, eventi. 

Tali informazioni possono essere salvate attraverso due tipi di database:
- **SQL** di default. Viene consigliato l'utilizzo di PostgreSQL in quanto viene ampiamente supportato da ThingsBoard.
- **Ibridi** salvando le *entities* in PostgreSQL e i dati dei sensori utilizzando Cassandra o TimescaleDB. 


## Entità e relazioni
### Tipologie di entità
Come è possibile vedere nella parte precedente, ThingsBoard contiene una serie di entità utili al funzionamento della piattaforma. Essi sono:
- **Tenant** profilo che identifica un singolo o un'organizzazione che compra o possiede dispositivi e asset. Associato al tenant possiamo avere una serie di clienti che possono visualizzare quanto predisposto dal tenant. 
- **Customer** profilo che consente l'accesso in visualizzazione ai dispositivi già configurati dal Tenant. Un cliente può quindi avere più utenti che può creare e più dispositivi e asset da visualizzare. 
- **User** profilo che accede solo a dashboard e gestisce entità.
- **Dispositivi** entità che producono dati in telemetria e che gestiscono comandi RPC.
- **Asset** entità astratte che possono essere collegate a dispositivi o altri asset *(veicolo, campo, ecc.)*
- **Entity View** da configurare se si vuole far visualizzare solo una parte di dispositivi e asset al profilo Costumer.
- **Alarms** eventi che notificano problemi / informazioni con dispositivi, asset o entità.
- **Dashboard** visualizzazione dei dispositivi IoT per la loro configurazione e la costruzione dell'interfaccia grafica.

### Tipologie di relazioni
Le relazioni collegano dispositivi, asset e altre entità presenti all'interno dello stesso profilo Tenant.
I tipi di relazione sono arbitrari, come:
- *Contiene*
- *Gestisce*
- *ecc.*

Queste relazioni vengo poi salvate nella parte di database ThingsBoard dedicata alle entità. 

![Entità/Relazioni](img/entita-relazioni.svg) 
*Esempio di schema entità/relazione utilizzato in ThingsBoard*

## Comunicazione con ThingsBoard
### Protocolli supportati per salvataggio
Per il salvataggio di dati su server ThingsBoard è possibile utilizzare chiamate API possibili con vari protocolli, come:
- **HTTP**
- **MQTT**
- **CoAP**
- **LwM2M**

### Esempio con HTTP
Di seguito vengono riportate alcune istruzioni per comunicare con il server ThingsBoard per il salvataggio e la richiesta dei dati.

#### URL del server
L'URL da utilizzare per la comunicazione con il server ThingsBoard è:
```http
http(s)://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/telemetry
```
dove:
- *$THINGSBOARD_HOST_NAME* è l'hostname o l'indirizzo IP. 
- *$ACCESS_TOKEN* token del dispositivo (presente una volta creato su ThingsBoard)

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
- **Message**: qualsiasi evento. Può essere un dato, una richiesta, una chiamata API, ecc.
- **Rule node**: funzione che agisce su un messaggio e può compiere trasformazioni o elaborazioni del dato.
- **Rule chain**: connessione tra nodi che permette di trasferire l'output di un nodo come input di un altro.

![*rule engine*](img/rule-engine.png)
*Esempio di rule engine configurabile in ThingsBoard*

#### Linguaggi per i Rule Node
I linguaggi utilizzati per i *Rule Node* sono:
- **TBEL**: linguaggio di ThingsBoard con struttura simile a Java
- **JavaScript**: linguaggio ampiamente conosciuto e compatibile per i nodi di ThingsBoard

#### Tipi predefiniti di messaggi
Esistono tipi predefiniti di messaggi intercettabili in ThingsBoard; sono documentati [qui](https://thingsboard.io/docs/user-guide/rule-engine-2-0/overview/#predefined-message-types).

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
curl -v -X POST -d @telemetry-data-with-ts.json https://demo.thingsboard.io/api/v1/$ACCESS_TOKEN/telemetry --header "Content-Type:application/json"
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

**N.B.** *È possibile inviare i dati in formato JSON con i comandi di POST, ma risulta più comodo utilizzare un file JSON per dati complessi con molte chiavi o oggetti. La soluzione adottata con il file JSON è ottimale per le necessità del progetto.*

#### GET via HTTP
Per ottenere attributi dal server ThingsBoard è possibile eseguire la seguente richiesta:
```http
curl -v -X GET http(s)://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/attributes?clientKeys=attribute1,attribute2&sharedKeys=shared1,shared2
```
dove:
- *$THINGSBOARD_HOST_NAME*: hostname o indirizzo IP del server
- *$ACCESS_TOKEN*: token del dispositivo
- *clientKeys*: attributi client da recuperare
- *sharedKeys*: attributi condivisi da recuperare

L'output tipico di una richiesta simile è il seguente:
```json
{"client":{"attribute1":"value1","attribute2":true}}
```

#### Update via HTTP
Per ricevere aggiornamenti sugli attributi è possibile effettuare la seguente richiesta:
```http
curl -v -X GET https://$THINGSBOARD_HOST_NAME/api/v1/$ACCESS_TOKEN/attributes/updates?timeout=20000
```
dove `timeout` indica il tempo in millisecondi per attendere un aggiornamento; trascorso il timeout la richiesta viene chiusa senza risultati.
Il formato di output è lo stesso di una normale richiesta GET.


## Link utili
- [Architettura ThingsBoard](https://thingsboard.io/docs/reference/)
- [Entità e relazioni](https://thingsboard.io/docs/user-guide/entities-and-relations/)
- [Comunicazione con ThingsBoard](https://thingsboard.io/docs/reference/protocols/)
- [HTTP API](https://thingsboard.io/docs/reference/http-api/)