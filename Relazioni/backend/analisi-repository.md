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
I file JSON contengono i dati che potrebbero essere inviati tramite le seguenti richieste.