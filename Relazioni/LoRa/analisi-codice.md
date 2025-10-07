# Analisi codice
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