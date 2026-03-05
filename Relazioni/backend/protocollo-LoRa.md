# Protocollo di comunicazione LoRa

La comunicazione con le arnie richiede un protocollo wireless, dati i requisiti del cliente. Le api sono sensibili a certe frequenze radio prodotte, quindi è necessario comunicare a una frequenza che non vada a disturbare il loro lavoro.  

La comunicazione avviene tra un Raspberry Pi con un LoRa HAT montato, che permette di comunicare con un microcontrollore basato su ESP32, a sua volta dotato di un chip LoRa, per l’invio delle letture eseguite attraverso i sensori ad esso collegati verso il Raspberry. Il Raspberry funge da gateway, di conseguenza si occupa di effettuare un instradamento dei dati verso un server ThingsBoard inserito all’interno della rete LAN, al quale accede per salvare le letture all’interno di un database e visualizzarle in una dashboard.  

Ciò impedisce la normale comunicazione attraverso un protocollo dello stack TCP/IP come il Wi-Fi 802.11. È quindi necessaria una comunicazione con parametri specifici, ove sia dimostrato che non vadano a influire con la corretta operatività delle arnie. Seguono i parametri di frequenza e potenza radio utilizzati da LoRa in Unione Europea.

## Parametri di comunicazione del protocollo LoRa nell’UE

Tutte le informazioni qui mostrate sono state ricavate dalle specifiche regionali di LoRa, a meno che non sia inserito un link di una sorgente diversa:  
[https://resources.lora-alliance.org/technical-specifications/rp002-1-0-4-regional-parameters](https://resources.lora-alliance.org/technical-specifications/ts001-1-0-4-lorawan-l2-1-0-4-specification)

([Riga 1266-1277, pagina 40, Specifiche generali del protocollo](https://resources.lora-alliance.org/technical-specifications/ts001-1-0-4-lorawan-l2-1-0-4-specification) - Il modulo non può in alcun caso superare i 36 dBm o 3.98 W, ma questi sono solo i valori massimi supportati dal modulo. Seguono le specifiche massime in Europa.)

(Sezione 2.4.2, Pagina 32, Tabella riga 572)

Il modulo utilizza la banda EU863-870 che ha le frequenze specificate dalla tabella: **868.10 MHz - 868.625 MHz**  
in base al canale che viene utilizzato per la comunicazione.

(Sezione 2.4.3, Pagina 33, Paragrafo Riga 595)

La potenza massima trasmessa in Europa deve essere di **+16 dBm (39.8 mW)**, secondo la banda europea EU863-870 (indicata anche nelle [specifiche regionali di questa banda](https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/eu868/) sotto la sezione "*Maximum EIRP / ERP*").

**ERP**: *Effective Radiated Power*, ovvero la potenza effettiva irradiata da un'antenna tenendo conto del suo guadagno rispetto a un dipolo standard. EIRP differisce da ERP perché il guadagno dell'antenna viene misurato rispetto a un'antenna isotropa.

# Protocollo ideale

Per soddisfare questi requisiti, il protocollo ideale è il LoRaWAN, ovvero un'integrazione layer 3 del protocollo LoRa, che invece si colloca al layer 2. Seguono le differenze fra i due e i problemi riscontrati relativi ad essi.

### Differenza tra LoRa e LoRaWAN

*Il protocollo LoRa è solamente un protocollo layer fisico della pila ISO/OSI; di conseguenza, gestisce solamente un broadcast via radio verso gli altri dispositivi compatibili con lo standard LoRa nelle vicinanze. Non gestisce alcuna tecnica a contesa per connessioni multiple, né connessioni o criptazione dei dati.* 

![](img/lora-protocol.png)

*Di conseguenza, se viene acquistato un modulo che supporta **solamente** il protocollo LoRa, e non LoRaWAN, non sarà possibile gestire una serie di funzionalità tra cui le seguenti:*  

- *Connessioni multiple attraverso un singolo dispositivo*  
- *Criptazione dei dati attraverso una chiave di applicazione*  
- *Classi di lavoro per diminuire la potenza utilizzata compromettendo la latenza*  

*Quando si stabilisce una connessione di un dispositivo con un gateway LoRaWAN, è quindi*  

*necessario specificare i seguenti parametri:*  

- ***EUI Dispositivo***: identificativo del dispositivo a cui si deve connettere il gateway  
- ***EUI Applicazione***: identificativo dell’applicazione utilizzata; su alcuni gateway, come il Milesight, basta impostarlo a tutti zeri  
- ***Chiave Applicazione***: chiave di criptazione utilizzata per cifrare il contenuto dei pacchetti  

*Nel caso in cui la connessione non venga eseguita attraverso OTAA (Over The Air Activation), che*  

*è un handshake che permette la trasmissione di ulteriori chiavi di comunicazione all’attivazione, viene utilizzata invece ABP (Activation By Personalization). Quest’ultima tecnica di connessione è meno sicura e richiede l’inserimento manuale di alcune chiavi ulteriori. Generalmente, però, è più altamente supportata rispetto alla più sicura OTAA. La maggior parte degli errori riscontrati durante l’utilizzo di schede di sviluppo LoRa sono stati provocati dall’OTAA.*

## Differenze fra chip

La differenza fra i due protocolli è stata evidenziata perché un problema riscontrato durante lo sviluppo è dato dal mancato supporto di un chip che integri LoRaWAN lato server, a basso costo e disponibile per le piattaforme richieste.

La comunicazione fra il Raspberry Pi e l’ESP32 avviene facendo uso di un chip SX1262. Malgrado questo supporti la comunicazione LoRaWAN solo come client (e non come server), da quanto risulta da varie fonti online, la comunicazione tra il chip ESP32 e un gateway Milesight LoRaWAN server è sempre fallita nonostante i vari tentativi, seguendo i passaggi riferiti dal manuale.

In ogni caso, per la corretta comunicazione tramite LoRaWAN, il gateway deve essere composto da un chip della serie SX123x, i quali non sono prodotti in massa su LoRaHAT per Raspberry Pi. Non è nemmeno possibile utilizzare un gateway commerciale come quello della Milesight utilizzato per le prove, perché non ci permetterebbe di gestire manualmente l’occupazione della banda radio che, data l’interferenza con le arnie, deve essere controllata.

## Scelta del chip

Il chip per la comunicazione è stato scelto sulla base del supporto di librerie. Il chip SX1262 è supportato dalla libreria [RadioLib](https://github.com/jgromes/RadioLib), che a sua volta supporta contemporaneamente sia la piattaforma Arduino che la piattaforma Raspberry Pi, permettendo di scrivere codice portabile su entrambe le piattaforme con modifiche minori.

# Creazione del protocollo di comunicazione

Date le problematiche citate in questo documento, è sembrata plausibile la creazione di un protocollo che permetta la comunicazione con una certa affidabilità, facendo solamente uso di radio LoRa.

Segue una descrizione delle caratteristiche del protocollo.

# Descrizione implementazioni protocollo

Il protocollo introduce le seguenti funzionalità alla comunicazione permessa dalla libreria `RadioLib`:

- Indirizzamento  
- Messaggi di controllo  
- Handshake fra mittente e destinatario con supporto per ritrasmissioni (implementando un limite sul massimo delle ritrasmissioni consentite)
- Invio e verifica della CRC32 durante la handshake per il controllo errori
- Frammentazione a layer superiore nel caso il pacchetto superi la MTU del chip utilizzato
- Logging robusto con due livelli differenti

Un pacchetto inviato dal protocollo contiene la sorgente e la destinazione del messaggio. Il dispositivo ricevente viene identificato con una stringa di testo che deve combaciare con quella contenuta nella destinazione del pacchetto ricevuto, altrimenti il pacchetto viene ignorato. Seguono le specifiche per la costruzione dell'identificatore:
# Identificatore
Secondo le specifiche del protocollo, l'identificatore deve avere sottostare alle seguenti caratteristiche:
  - Deve avere una lunghezza compresa fra 4 e 12 caratteri
  - Deve essere alfanumerico, nessun carattere speciale consentito in quanto potrebbe andare ad interferire con i separatori che vengono aggiungi a layer 2
  - Non deve essere l'identificatore di broadcast, definito all'interno della libreria come la stringa `BROAD`. Questo identificatore non può essere impostato come identificatore di alcun nodo, ma può solamente essere una destinazione

Nell’header, in seguito alla sezione contenente sorgente e destinazione del pacchetto, segue un valore numerico contenente il messaggio di controllo, che può essere un `SYN`, un `ACK`, oppure un `RETRANSMISSION_TIMEOUT`. Questi messaggi di controllo vengono utilizzati per identificare la funzione dei pacchetti durante la handshake.
# Handshake a 3 vie e ritrasmissioni
Il protocollo implementa una handshake a 3 vie. Questa viene utilizzata ad ogni invio di ogni pacchetto. Se un pacchetto viene frammentato, ogni frammento verrà inviato utilizzando questa handshake, si illustrano le fasi di questa:
- Il mittente invia un `SYN` al destinatario, contenente come payload la stringa da recepire
- Il destinatario, alla ricevuta del `SYN`, calcola il CRC32 del suo payload e risponde con un `ACK`, il cui payload sarà a sua volta riempito con il risultato di questo calcolo
- Il mittente procede a verificare la CRC32 ricevuta dal destinatario comparandola con quella da lui calcolata sul messaggio originale. Se le due combaciano, allora procede a rispondere con un `ACK` finale di conferma con payload vuoto, altrimenti viene inviato un nuovo `SYN`. All'invio del nuovo `SYN` viene effettuata una ritrasmissione che causa la ripetizione della handshake.
- Se vengono effettuate piú di `MAX_RETRANSMISSIONS` ritrasmissioni, allora il mittente invia un pacchetto di tipo `RETRANSMISSION_TIMEOUT` indicando che é stato raggiunto il numero massimo di queste. Se questo pacchetto viene recepito correttamente al destinatario, allora questo interromperà la ricezione restituendo un codice di errore. Altrimenti sarà necessario attendere un timeout.

# Funzionalità di basso livello
Ogni volta che viene inviato un pacchetto attraverso `send` o `receive`, in background viene effettuato un handshake a tre vie fra il dispositivo e la destinazione tramite funzioni di livello più basso, implementate privatamente all'interno della libreria, che possono essere denominate `send_raw` e `receive_raw`. Queste funzioni "raw" si occupano esclusivamente di incapsulare tre informazioni, all'interno di un vettore di dati grezzo, che può essere inviato al layer 1 verso l'esterno tramite segnali radio LoRa in broadcast
Queste informazioni sono le seguenti: 
    - ID del dispositivo e della destinazione
    - Messaggio di controllo 
    - Corpo effettivo del pacchetto

# Classe per la conversione in JSON dei dati
È stata implementata una classe per gestire la conversione in JSON dei dati inviati verso il lato ricevente, al fine di semplificarne l'invio a ThingsBoard, sebbene tale funzionalità non sia ancora stata effettivamente implementata dal punto di vista pratico.
La classe in questione si chiama **Measure** e contiene i seguenti attributi:
- *measureType* grandezza fisica misurata
- *value* valore di misurazione (per default viene impostata come tipo float)
- *timestamp* timestamp della misurazione nel momento in cui è stata effettuata

```c++
private:
    char measureType[20];
    float value;
    unsigned long timestamp;
```

All'interno contiene il metodo per la conversione in una stringa JSON con un formato adattato a ThingsBoard. La funzione è la seguente:
```c++
    /**
     * function to convert the measure to a JSON string
     * @return JSON string with the data of the measure
     */
    const char *toJSON()
    {
        static char buffer[MAX_LENGTH_JSON_STRING];
        sprintf(buffer, "{\"ts\":%lu, \"values\"={\"%s\"=%f}}", timestamp, measureType, value);
        return buffer;
    }
```

Nonostante non dovrebbe essere necessario per il ricevitore, è presente anche una funziona statica per la conversione della stringa in formato JSON in un oggetto **Measure**. 

# Frammentazione
Il protocollo implementa un fragmenter che gestisce la frammentazione (`FragmentConstructor`) e la deframmentazione (`FragmentDestructor`) dei pacchetti. Segue una descrizione della gestione della frammentazione:
- **Invio**:
    - Alla chiamata della funzione `send(str, str)`, viene utilizzato il `FragmentConstructor` per separare il messaggio in segmenti, ognuno con header che separa:
        - Fragment Index, indice del frammento
        - Total Fragments, ammontare totale di frammenti
        - Payload, messaggio frammentato
    - Ogni frammento viene individualmente inviato via radio effettuando la handshake con le dovute ritrasmissioni. In caso di errore irreversibile nell'invio o nella costruzione di un frammento, l'invio viene interrotto e l'errore viene propagato alla funzione `send(str, str)`
- **Ricezione**
    - Alla chiamata della funzione `receive(str*, str*)`, viene utilizzato il `FragmentDestructor` per ricostruire i frammenti ricevuti, il quale può produrre alcuni errori, quali:
        - Ricezione di frammenti con indici non sequenziali o ordinati
        - Frammenti con header invalidi
        - Frammenti con valori incongruenti fra loro
        - Header malformati che causano errori di parsing
    - Uno qualsiasi di questi errori causa il ritorno del valore `NIAGARA_INVALID_FRAGMENT` dalla funzione.
    - La libreria gestisce anche il contesto dell'invio dei pacchetti frammentati. Viene salvata la sorgente del primo pacchetto frammentato ricevuto dalla funzione `receive(str*, str*)`. Questa sorgente viene verificata ad ogni ricezione di pacchetti frammentati successivi. Qualora avvenga che pacchetti frammentati vengano inviati al destinatario di un altro flusso di pacchetti frammentati non terminato, i primi verranno ignorati fino a quando il destinatario non ha cessato la ricezione e ricostruzione del messaggio originale.


## Utilizzo della libreria

- **Costruttore**  
  Il costruttore della libreria permette la creazione e gestione del logger
  `Niagara(log_handler, Niagara_LogLevel::TERMINAL);`
  - Il primo parametro è il puntatore ad una funzione di firma:
    `void funzione(const char*)`
    Questa funzione gestisce i messaggi di log prodotti dalla libreria. Astrae l'output del log dalla libreria stessa, permettendo la definizione di un output qualunque, che esso sia un file, un display o una seriale di uscita. Ció è indipendente dalla piattaforma.
  - Il secondo parametro definisce il livello di log. Qualora la funzione handler del logger stampi in output su un display di un microcontrollore a dimensioni ridotte, il logger può essere impostato con livello `Niagara_LogLevel::DISPLAY` per produrre log output piú concisi, altrimenti si può utilizzare `Niagara_LogLevel::TERMINAL` per un log completo.

- **Impostazione Identificatore**
  Dopo aver chiamato il costruttore, è necessario impostare l'identificatore del dispositivo prima di poter chiamare i metodi di invio e ricezione:
  `bool set_identifier(str identifier);`
  La chiamata di questo identificatore provoca un controllo di verifica dei requisiti sull'identificatore passato. Se questo controllo va a buon fine, la funzione restituisce `true` e imposta l'identificatore nel dispositivo corrente.
  La mancata chiamata di questo metodo prima di effettuare chiamate a funzioni di invio o ricezione provoca il ritorno di `NIAGARA_NO_IDENTIFIER` da queste.

- **Invio dati**  
  L’invio dei dati avviene attraverso l’utilizzo del seguente metodo:  
  `Niagara_Ret Niagara::send(str destination, str message);`
  Si specifica come prima stringa la destinazione del pacchetto, ovvero l’identificativo o il *callsign* che il dispositivo ricevente ha inserito nel costruttore della libreria. Successivamente viene specificato il messaggio da recapitare.
  Nel caso ci sia stato un errore durante l’invio dei dati, viene ritornato un valore diverso da `NIAGARA_OK` contenuto nell'enumeratore `Niagara_Ret`.
  La classe `str` definita in `str.h`, implementa un oggetto stringa compatibile minimalmente sia con `std::string` che con `String` di Arduino. Altri tipi di stringhe devono essere convertiti in questa classe prima di poterli passare al protocollo Niagara.

- **Ricezione dati**  
  La ricezione dei dati avviene attraverso il metodo bloccante seguente:  
  `Niagara_Ret Niagara::receive(str* source, str* output);`
  Richiede un puntatore a una stringa che verrà riempita con l’identificatore della sorgente che ha inviato il messaggio. Mentre un'altra stringa viene popolata con il messaggio ricevuto effettivo.
  Nel caso ci sia stato un errore durante la ricezione dei dati, viene ritornato un valore diverso da `NIAGARA_OK` contenuto nell'enumeratore `Niagara_Ret`.

## Criteri minimi di validazione
Nella prima iterazione del protocollo, il minimo indispensabile per consentire la comunicazione è rappresentato dalle funzioni di basso livello, quali `receive_raw` e `send_raw`, che vengono utilizzate dalle funzioni pubbliche di livello più alto `receive` e `send` all'interno della libreria durante l'handshake.

Queste funzioni si occupano esclusivamente dell'incapsulamento dei parametri di invio in una singola sequenza di dati destinata all'invio in broadcast radio. In fase di ricezione, si occupano invece del deincapsulamento dei dati e del controllo della stringa di destinazione, al fine di verificare che il dispositivo di destinazione corrisponda a quello corrente. Quest'ultima operazione, data la sua complessità, è quella che al momento causa problemi, rendendo impossibile il test completo del protocollo.

## Miglioramenti futuri
Si prevede di integrare il protocollo con le seguenti funzionalità:

- Fare in modo che la funzione di invio dati costruisca direttamente il JSON da inviare nel corpo del messaggio destinato al gateway, in modo da passare direttamente le letture dei sensori e lasciare che essa si occupi della loro formattazione, funzione che al momento esiste nel codice, ma non e' stata ancora implementata.
