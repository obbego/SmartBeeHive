# Analisi repository
La seguente parte di relazione serve a indicare le funzionalità del codice presente in questa repository, per la parte backend dell'alveare. 

**N.B.** *In questa repository non sono stati aggiunti alcuni codici di prova iniziati nel progetto. Tali file erano scritti per Arduino contenenti i test dei sensori e la prima prova completa di un modulo ESP32.*


## Protocollo Lora
```
repository/
    └── niagara/
           ├── hal/ //libreria contenente librerie per Raspberry
           ├── main.cpp
           ├── niagara.cpp
           └── niagara.h
```
I file indicati contengono la libreria e un file di test per un protocollo personalizzato costruito al di sopra del LoRa. L'esigenza è quella di inviare dati di tipologia diversa in modo da essere recepiti dal gateway e successivamente inviati al database dove poterli salvare. 

**N.B.** La repository è stata aggiornata con nuovi moduli che astraggono varie funzionalità dell'invio di telemetrie tra dispositivi. Consultare la cartella indicata per poter visualizzare l'elenco completo dei file e delle funzionalità, di cui è presente una documentazione accurata all'interno del codice. 

## Thingsboard
### Rule chains
```
repository/
    └── thingsboard/
           └── rule-chains/
                └── *.json
```
I file delle rule chains sono dei file di configurazione inseriti su ThingsBoard dove si eseguono controlli o azioni basandosi su condizioni applicate ai dati in input (telemetrie, richieste ecc.).
Tali configurazioni vengono salvate in file JSON e sono facilmente trasferibili da un server all'altro. 

### Scheduler
```
repository/
    └── thingsboard/
           └── scheduler/
                ├── launch_background-script.sh
                ├── stop_background-script.sh
                ├── rpc-scheduler.py
                ├── device-register.json
                └── timing-scheduler.json
```
Lo scheduler per l'invio di richieste RPC periodiche a ThingsBoard contiene i seguenti file rilevanti:
- **launch_background-script.sh** file shell per distribuzioni Linux per avviare il codice in background. Per modificare il nome del file da eseguire o il suo percorso agire nella prima parte del file, nel variabile relativa inserita. Se non è presente installa l'ambiente Python nel dispositivo. 
- **stop_background-script.sh** file sheel per distribuzioni Linux per fermare lo script precedentemente avviato in background. Per modificare il nome del file da fermare agire sempre sulla variabile indicata all'inizio della lista dei comandi. 
- **rpc-scheduler.py** Codice Python per la richiesta RPC al Server ThingsBoard. Non contiene moduli aggiuntivi. 
- **device-register.json** File JSON utilizzato dallo script Python per reperire all'inizio la lista dei file ai quali effettuare le modifiche. La struttura del file si presenta così: 
```json
[
    {
        "name":"device_1",
        "access-token":"access_token_1"
    },
    {
        "name":"device_1",
        "access-token":"access_token_1"
    }
]
```
Dove il nome del device serve solo per una migliore identificazione dell'utente del dispositivo (non influisce in alcun modo nelle richieste che poi andranno ad effettuarsi), mentre l'access token deve corrispondere a quello del device configurato su ThingsBoard.
- **timing-scheduler.json** file utilizzato dallo script Python che contiene i vari orari per richiesta RPC in cui lo scheduler deve interpellare il server ThingsBoard. La struttura del file si presenta così:
```json
{
    "task_1":[
        {"hour":18,"minute":38}
    ],
    "task_2":[
        {"hour":18,"minute":39}
    ]
}
```
Il nome dei task devono essere quelli identificati nelle codice. L'orario in ore e minuti rispetta il fuso orario locale e può esserne aggiunto più di uno, secondo le esigenze.

## Test
### Richieste HTTP
```
repository/
    └── test/
           └── request-http/
                ├── *.bat
                ├── *.sh
                └── *.json
```
Questa repository contiene i file shell / batch per richieste inviate al server ThingsBoard.
Tali comandi possono riguardare il salvataggio di una telemetria o di una richiesta RPC da far eseguire al server.
I file JSON contengono i dati che potrebbero essere inviati tramite le seguenti richieste.à

## Receiver
```
repository/
    └── receiver/
            ├── devices.txt
            ├── lora-receiver.cpp
            ├── start_lora-receiver.sh
            └── stop_lora-receiver.sh
```
La cartella indicata contiene il codice per il funzionamento del ricevitore LoRa collegato poi a ThingsBoard. 
**Il codice necessita per essere avviato della libreria Niagara** sviluppata in questo progetto. 
La cartella è costituita dai seguenti file:
- **devices.txt** file contenente la lista di dispositivi a cui è collegato il ricevitore che rispetta il seguente formato:
```
DEVICE_1 TOKEN_1
DEVICE_2 TOKEN_2
...
```
dove il primo parametro è il nome dell'identificatore dell'arnia utilizzato dal protocollo Niagara, mentre il secondo è l'access token di ThingsBoard. Non è necessario che l'identificatore Niagara corrisponda con il nome del dispositivo dato su ThingsBoard. 
- **lora-receiver.cpp** codice contenente il funzionamento del ricevitore LoRa. Questo codice è studiato per dispositivi come Raspberry Pi, ai quali è possibile collegare un ricevitore LoRa. A parte la libreria di progetto, il codice non prevede l'implementazione di altri moduli realizzati dal gruppo di lavoro. 
- **start_lora-receiver.sh** script shell per l'avvio del ricevitore in background. In caso non fossero presenti, lo script prevede l'installazione automatica delle librerie necessarie per il funzionamento del codice. 
- **stop_lora-receiver.sh** script shell che permette di terminare il programma precedentemente lanciato in background.

## Arnia ESP32
```
repository/
    └── sender_esp32/
            ├── sender_esp32.ino
            └── libraries-info.txt
```
Questo cartella contiene il codice di funzionamento della scheda ESP32 collegata all'arnia per l'invio delle telemetrie.
La cartella contiene quindi i seguenti file:
- **sender_esp32.ino** con il codice per il funzionamento compatibile con Arduino IDE. A parte le librerie da installare nel codice e alla libreria di progetto, non si collega ad altri moduli realizzati dal gruppo di lavoro. 
- **libraries-info.txt** file con le indicazioni sui nomi delle librerie da installare. Sono quindi delle semplici indicazioni per il configuratore che non hanno validità nel codice. 