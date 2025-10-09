# Protocollo di comunicazione LoRa 
La comunicazione con le arnie richiede un protocollo wireless dati i requisiti del cliente. Le api sono sensibili a certe frequenze radio prodotte quindi e’ necessario comunicare ad una frequenza che non vada a disturbare il loro lavoro.  

La comunicazione avviene tra un Raspberry Pi con un LoRa HAT montato, che permette di comunicare con un microcontrollore basato su ESP32 con a sua volta un chip LoRa montato per l’invio delle letture eseguite attraverso i sensori ad esso collegati verso il raspberry. Il raspberry funge da gateway, di conseguenza si occupa di effettuare un instradamento dei dati verso un server thingsboard inserito all’interno della rete LAN a cui accede per salvare le letture all’interno di un database e visualizzarle in una dashboard. 

Cio’ impedisce la normale comunicazione attraverso un protocollo dello stack TCP-IP come il WiFi 802.11. E’ quindi necessario una comunicazione con parametri specifici, ove sia provato che non vadano ad influire con la corretta lavorazione delle arnie. Seguono i parametri di frequenza e potenza radio utilizzati da LoRa in unione europea. 
## Parametri di comunicazione del protocollo LoRa nell’UE 
Tutte le informazioni qua mostrate sono state ricavate dalle specifiche regionali di LoRA, almeno che non sia inserito un link di una sorgente diversa:  [https://resources.lora-alliance.org/technical-specifications/rp002-1-0-4-regional-parameters](https://resources.lora-alliance.org/technical-specifications/ts001-1-0-4-lorawan-l2-1-0-4-specification) 

([Riga 1266-1277, pagina 40, Specifiche generali del protocollo](https://resources.lora-alliance.org/technical-specifications/ts001-1-0-4-lorawan-l2-1-0-4-specification) - Il modulo non può in alcun caso superare i 36dBm o 3.98W, ma questi sono solo i valori massimi supportati dal modulo, seguono le specifiche massime in Europa) 

(Sezione 2.4.2, Pagina 32, Tabella riga 572) 

Il modulo utilizza la banda EU863-870 che ha le frequenze specificate dalla tabella: **868.10MHz - 868.625MHz**  

In base al canale che viene utilizzato per la comunicazione. 

(Sezione 2.4.3, Pagina 33, Paragrafo Riga 595) 

La potenza massima trasmessa in Europa deve essere di **+16dBm (39.8mW)**, secondo la banda europea EU863-870 (Indicata anche nelle [specifiche regionali di questa banda](https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/eu868/) sotto la sezione "*Maximum EIRP / ERP*") 

*ERP*: **Effective Radiated Power**, ovvero la potenza effettiva irradiata da un'antenna tenendo conto del suo guadagno rispetto ad un dipolo standard, EIRP differisce da ERP perché il guadagno dell'antenna viene misurato rispetto ad un'antenna isotropa. 
# Protocollo ideale 
Per la soddisfazione di questi requisiti, il protocollo ideale e’ il LoRaWAN, ovvero un'integrazione layer 3 del protocollo LoRa, che sarebbe layer 2. Seguono le differenze fra i due ed i problemi riscontrati relativi ad essi.

*Differenza tra LoRa e LoRaWAN* 

*Il protocollo LoRa è solamente un protocollo layer fisico della pila ISO/OSI, di conseguenza gestisce solamente un broadcast via radio verso gli altri dispositivi compatibili con lo standard LoRa nelle vicinanze. Non gestisce alcuna tecnica a contesa per connessioni multiple, non gestisce nemmeno connessioni o criptazione dati.* 

![](Aspose.Words.de22a184-0d72-48d9-a225-55b0e886d0af.001.jpeg)

*Di conseguenza, se viene acquistato un modulo che supporta **solamente** il protocollo LoRa, e non LoRaWAN, non sarà possibile gestire una serie di funzionalità tra cui le seguenti:* 

- *Connessioni multiple attraverso un singolo dispositivo* 
- *Criptazione dati attraverso una chiave di applicazione* 
- *Classi di lavoro per diminuire la potenza utilizzata compromettendo la latenza* 

*Quando si stabilisce una connessione di un dispositivo con un gateway LoRaWAN, è quindi* 

*necessario specificare i seguenti parametri:* 

- ***EUI Dispositivo**: sarebbe l’identificativo del dispositivo a cui si deve connettere il gateway* 
- ***EUI Applicazione**: identificativo dell’applicazione utilizzata, su alcuni gateway come il Milesight basta impostarlo a tutti zeri* 
- ***Chiave Applicazione**: chiave di criptazione che viene utilizzata per cifrare il contenuto dei pacchetti* 

*Nel caso la connessione non venga eseguita attraverso OTAA (Over The Air Activation), che* 

*è una handshake che permette la trasmissione di ulteriori chiavi di comunicazione all’attivazione, viene utilizzata, invece, ABP (Activation By Personalization). Quest’ultima tecnica di connessione è meno sicura e richiede l’inserimento manuale di alcune chiavi ulteriori. Generalmente, però, è più altamente supportata della più sicura OTAA. La maggior parte degli errori riscontrati durante l’utilizzo di schede di sviluppo LoRa sono stati provocati dall’OTAA.* 
## Differenze fra chip 
La differenza fra i due protocolli e’ stata dimostrata perche’ un problema riscontrato durante lo sviluppo e’ dato dal mancato supporto di un chip che integri il LoRaWAN lato server a basso costo e disponibile per le piattaforme richieste. 

La comunicazione fra il Raspberry Pi e l’ESP32 avviene facendo uso di un chip SX1262, malgrado questo supporti solamente la comunicazione LoRaWAN come client, e non come server, da quello che e' stato letto da varie fonti su internet. Nonostante ciò, in ogni caso la comunicazione tra il chip ESP32 ed un gateway Milesight LoRaWAN server e’ sempre fallita nonostante i vari tentativi seguendo i passaggi riferiti dal manuale. 

In ogni caso, per la corretta comunicazione tramite LoRaWAN, il gateway deve essere composto da un chip della serie SX123x, i quali non sono prodotti in massa su LoRaHAT per Raspberry Pi. Non e’ nemmeno possibile utilizzare un gateway commerciale come quello della Milesight utilizzato per le prove, perche’ non ci permetterebbe di gestire manualmente l’occupazione della banda radio che, data l’interferenza con le arnie, deve essere controllata. 
## Scelta del chip 
Il chip per la comunicazione è stato scelto sulla base del supporto di librerie. Il chip SX 1262 e’ supportato dalla libreria [RadioLib](https://github.com/jgromes/RadioLib) che a sua volta supporta contemporaneamente sia la piattaforma Arduino che la piattaforma Raspberry Pi, permettendo di scrivere codice portabile su entrambe le piattaforme con modifiche minori. 
# Creazione del protocollo di comunicazione 
Date le problematiche citate in questo documento, e’ sembrata plausibile la creazione di un protocollo che permetta la comunicazione con una certa affidabilita’ facendo solamente uso di LoRa radio. 

Segue una descrizione delle caratteristiche del protocollo 
# Descrizione protocollo 
Il protocollo introduce le seguenti funzionalità alla comunicazione corrente: 

- Indirizzamento 
- Messaggi di controllo 
- Abilitazione del CRC sul protocollo LoRa sottostante 

Un pacchetto inviato dal protocollo contiene la sorgente e la destinazione del messaggio, il dispositivo ricevente viene identificato con una stringa di testo che deve combaciare con quella contenuta nella destinazione del pacchetto ricevuto, altrimenti il pacchetto viene ignorato, la stringa di testo non ha un limite sulla lunghezza al momento. 

Nell’header, in seguito alla sezione contenente sorgente e destinazione del pacchetto segue un valore numerico contenente il messaggio di controllo, che può essere una REQUEST, una REPLY o un PING, al momento il protocollo non ha comportamenti differenti in base al messaggio di controllo inviato, esso può essere direttamente specificato e letto dall’utente. Il protocollo inoltre si occupa di abilitare la codifica CRC nel LoRa sottostante per garantire una maggiore accuratezza nel trasferimento delle informazioni. 
## Utilizzo della libreria 
- **Costruttore** 

  Il costruttore della libreria permette la creazione dell’identificatore del dispositivo: NiagaraPi(“RASPI”, true); 

  Il secondo parametro e’ opzionale e determina se mostrare i messaggi di log nel terminale durante l’inizializzazione del modulo radio. 

- **Invio dati** 

  L’invio dei dati avviene attraverso l’utilizzo del seguente metodo: 

  Niagara\_Ret NiagaraPi::send(std::string destination, Niagara\_Control control, std::string message); 

  Si specifica come prima stringa la destinazione del pacchetto, ovvero l’identificativo che il dispositivo ricevente ha inserito nel costruttore della libreria, successivamente si specifica il messaggio di controllo, contenuto nell’enumeratore Niagara\_Control e infine viene specificata una stringa con il messaggio vero e proprio. 

  Nel caso ci sia stato un errore durante l’invio dei dati, viene ritornato un valore diverso da NIAGARA\_OK contenuto nell'enumeratore Niagara\_Ret. 

- **Ricezione dati** 

  La ricezione dei dati avviene attraverso il metodo bloccante seguente: Niagara\_Ret NiagaraPi::receive(std::string\* source, Niagara\_Control\* control\_output, std::string\* message\_output); 

  Richiede un puntatore ad una stringa che verra’ riempita con l’identificatore della sorgente che ha inviato il messaggio, inoltre un altro puntatore ad un tipo Niagara\_Control che riempira’ il valore di controllo che conteneva il messaggio ricevuto e infine un ultima stringa che viene popolata con il messaggio ricevuto effettivo. 

  Nel caso ci sia stato un errore durante la ricezione dei dati, viene ritornato un valore diverso da NIAGARA\_OK contenuto nell'enumeratore Niagara\_Ret. 
## Miglioramenti futuri 
Si prevede di integrare il protocollo con le seguenti funzionalità: 

- Handshake tra i dispositivi e gestione (benchè minima) delle sessioni a layer 4, con aggiunta di ritrasmissione dei pacchetti 
- Messaggi di controllo Utilizzati effettivamente per questo handshake 
- Specificare un formato per le stringhe di indirizzamento 
- Fare in modo che la funzione di invio dati costruisca direttamente il json da inviare nel corpo del messaggio destinato al gateway in modo da passare direttamente le letture dei sensori ad essa e lasciare che essa si occupi della formattazione di queste 
# Specifiche protocollo 
## Handshake di inizio connessione 
Per instaurare una connessione da un dispositivo A (possibilmente il gateway) ad un dispositivo B (il ricevitore della scheda sensori) 

- GATEWAY INVIA SYN cioè chiede di connettersi con il dispositivo interessato utilizzando un identificativo specifico. 
- ACKNOWLEDGEMENT dal dispositivo che riceve la richiesta, confermando la connessione 
- DISPOSITIVO INVIA UN PING chiedendo una conferma di connessione al primo dispositivo 
- DISPOSITIVO CONFERMA mandando un pacchetto di acknowledgement. 

Tale operazione verrà ripetuta ogni qualvolta si instaura una connessione tra due dispositivi.  

Le funzioni che la libreria dovrebbe prevedere sono: 

- *connect()* per il dispositivo che instaura una connessione.  
- *listen()* che blocca il dispositivo fin quando non riceve una richiesta di sincronizzazione. 
## Conoscenza del tempo 
Nei dispositivi imputati alla rilevazione dei dati dei sensori (siano essi Arduino o ESP32) non è possibile sapere il giorno o l’ora di misurazione, ma è possibile sapere da quanti millisecondi la scheda è accesa.  

Il tempo però risulta essere un componente fondamentale. Infatti la connessione tra dispositivi via LoRa può essere fatta solo poche volte al giorno (tre indicativamente) per non interferire con le api e poterle danneggiare. Tuttavia la lettura dei sensori può non essere fatta solo al momento, ma includere anche altre eseguite durante il corso della giornata.  

Il tempo aiuta a comprendere quindi a identificare univocamente il tipo di misurazione assieme al tipo di valore misurato. Grazie a questo è possibile conservarla in memoria per una certa unità di tempo e poi scartata.  

Proprio per questo, tra le varie tipologie di comunicazione è *TIME\_SYNC.* Il pacchetto in questo caso trasmette il timestamp della data precisa o i millisecondi dall’accensione della scheda.  

Così è possibile che i dispositivi possano risalire all’ora in cui le misurazioni sono state compiute. 

**N.B.** Tale comunicazione è completamente slegata dall’handshake iniziale e può essere fatta in qualsiasi momento sia opportuno, anche alla riaccensione di uno dei due dispositivi. 