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

## Creazione del protocollo di comunicazione

Date le problematiche citate in questo documento, è sembrata plausibile la creazione di un protocollo che permetta la comunicazione con una certa affidabilità, facendo solamente uso di radio LoRa.

Segue una descrizione delle caratteristiche del protocollo.

# Descrizione implementazioni protocollo

Il protocollo introduce le seguenti funzionalità alla comunicazione permessa dalla libreria `RadioLib`:

- Stack FIFO per la lettura dei dati ricevuti in maniera asincrona e non bloccante, facendo uso del multithreading su Raspberry Pi e della funzionalitá dual-core del chip ESP32 contenuto nella scheda di sviluppo.
- Indirizzamento  
- Messaggi di controllo  
- Handshake fra mittente e destinatario con supporto per ritrasmissioni (implementando un limite sul massimo delle ritrasmissioni consentite)
- Invio e verifica della CRC32 durante la handshake per il controllo errori
- Frammentazione a layer superiore nel caso il pacchetto superi la MTU del chip utilizzato
- Logging robusto con due livelli differenti
- Layer applicativo per la gestione di misurazioni come PDU e correzione di timestamp per i dispositivi IoT che non dispongono di un modulo RTC

Un pacchetto inviato dal protocollo contiene la sorgente e la destinazione del messaggio. Il dispositivo ricevente viene identificato con una stringa di testo che deve combaciare con quella contenuta nella destinazione del pacchetto ricevuto, altrimenti il pacchetto viene ignorato. Seguono le specifiche per la costruzione dell'identificatore:
## Identificatore
Secondo le specifiche del protocollo, l'identificatore deve sottostare alle seguenti caratteristiche:
  - Deve avere una lunghezza compresa fra 4 e 12 caratteri
  - Deve essere alfanumerico, nessun carattere speciale consentito in quanto potrebbe andare ad interferire con i separatori che vengono aggiungi a layer 2

Non viene predisposto un identificatore di broadcast in quanto il protocollo prevede solamente comunicazioni unicast verso i dispositivi.

## Funzionalità di basso livello
Ogni volta che viene inviato un pacchetto attraverso `send` o `receive`, in background viene effettuato un handshake a tre vie fra il dispositivo e la destinazione tramite funzioni di livello più basso, implementate privatamente all'interno della libreria, che possono essere denominate `send_raw` e `receive_raw`. 
Queste funzioni "raw" si occupano esclusivamente di incapsulare tre informazioni, all'interno di un vettore di dati grezzo, che può essere inviato al layer 1 verso l'esterno tramite segnali radio LoRa in broadcast
Queste informazioni sono le seguenti: 
- ID del dispositivo e della destinazione
- Messaggio di controllo 
- Corpo effettivo del pacchetto
Nell’header, i messaggi di controllo, che può essere un `SYN`, un `ACK`, oppure un `RETRANSMISSION_TIMEOUT` vengono utilizzati per identificare la funzione dei pacchetti durante la handshake.

## Handshake a 3 vie e ritrasmissioni
Il protocollo implementa una handshake a 3 vie. Questa viene utilizzata all'invio di ogni pacchetto. Se un pacchetto viene frammentato, ogni frammento verrà inviato utilizzando questa handshake, si illustrano le fasi di questa:
- Il mittente invia un `SYN` al destinatario, contenente come payload la stringa da recepire
- Il destinatario, alla ricevuta del `SYN`, calcola il CRC32 del suo payload e risponde con un `ACK`, il cui payload sarà a sua volta riempito con il risultato di questo calcolo
- Il mittente procede a verificare la CRC32 ricevuta dal destinatario comparandola con quella da lui calcolata sul messaggio originale. Se le due combaciano, allora procede a rispondere con un `ACK` finale di conferma con payload vuoto, altrimenti viene inviato un nuovo `SYN`. All'invio del nuovo `SYN` viene effettuata una ritrasmissione che causa la ripetizione della handshake.
- Se vengono effettuate piú di `MAX_RETRANSMISSIONS` ritrasmissioni, allora il mittente invia un pacchetto di tipo `RETRANSMISSION_TIMEOUT` indicando che é stato raggiunto il numero massimo di queste. Se questo pacchetto viene recepito correttamente al destinatario, allora questo interromperà la ricezione restituendo un codice di errore. Altrimenti sarà necessario attendere un timeout.
    
## Frammentazione
Il protocollo implementa un fragmenter che gestisce la frammentazione (`FragmentConstructor`) e la deframmentazione (`FragmentDestructor`) dei pacchetti. Segue una descrizione della gestione della frammentazione:
- **Invio**:
    - Alla chiamata della funzione `send(str, str)`, viene utilizzato il `FragmentConstructor` per separare il messaggio in pacchetti, ognuno con header che separa:
        - Fragment Index, indice del frammento
        - Total Fragments, ammontare totale di frammenti
        - Payload, messaggio frammentato
    - Ogni frammento viene individualmente inviato via radio effettuando la handshake con le dovute ritrasmissioni.
    - Per la gestione degli errori nella frammentazione del messaggio o asincronismi di ricezione (es. dispositivo remoto che inizia la ricezione del terzo frammento quando il dispositivo corrente sta inviando il quarto perché il terzo frammento ha avuto un errore di trasmissione) a seguito dell'invio di ogni frammento il dispositivo remoto deve rispondere con una conferma che contiene l'indice del frammento da lui ricevuto e il numero di frammenti totali. Nel caso ci sia stato un errore nella deframmentazione o nel contenuto del pacchetto, viene inviato direttamente un codice di errore. Nel caso il `FragmentConstructor` legge un codice di errore o dei parametri di frammentazione ritornati invalidi, viene riavviata da capo la trasmissione del messaggio ripartendo dal primo frammento. Ad ogni pacchetto frammentato inviato tramite handshake nel layer di trasporto segue quindi un pacchetto di risposta, con la relativa handshake.
- **Ricezione**
    - Alla chiamata della funzione `receive(str*, str*)`, viene utilizzato il `FragmentDestructor` per ricostruire i frammenti ricevuti, il quale può produrre alcuni errori, quali:
        - Ricezione di frammenti con indici non sequenziali o ordinati
        - Frammenti con header invalidi
        - Frammenti con valori incongruenti fra loro
        - Header malformati che causano errori di parsing
    - Ad ogni frammento passato al deframmentatore, viene prodotto un pacchetto di ritorno che viene inviato tramite il layer di trasporto verso il mittente. Questo pacchetto contiene l'indice dell'ultimo frammento ricevuto ed il numero di frammenti totali. Nel caso uno di questi errori elencati precedentemente avvenga, un pacchetto contenente un codice di errore viene inviato come risposta. Nel caso il mittente percepisce una incongruenza nella comunicazione dei frammenti, il messaggio viene ritrasmesso da capo ripartendo dal primo frammento.
    - La libreria gestisce anche il contesto dell'invio dei pacchetti frammentati. Viene salvata la sorgente del primo pacchetto frammentato ricevuto dalla funzione `receive(str*, str*)`. Questa sorgente viene verificata ad ogni ricezione di pacchetti frammentati successivi. Qualora avvenga che pacchetti frammentati vengano inviati al destinatario di un altro flusso di pacchetti frammentati non terminato, i primi verranno ignorati fino a quando il destinatario non ha cessato la ricezione e ricostruzione del messaggio originale.

## Layer applicativo
Al di sopra del protocollo Niagara, viene implementato il layer applicativo tramite `niagara_measure.h`, che come PDU fa uso di oggetti `NiagaraMeasure`, contenenti:
- *Chiave*: Una stringa cross-platform `str` contenente la chiave della misura (es. `"temperature"`)
- *Valore*: Il valore della misura (es. `22.5`)
- *Timestamp*: Il momento in cui la misura è stata effettuata.

Dato che il dispositivo IoT che costruisce questi oggetti non dispone di un chip RTC, per ottenere correttamente il tempo di invio della misura questo layer si occupa di inviare nel messaggio sia il timestamp impostato dentro `NiagaraMeasure`, sia il timestamp indicativo di quando la misura è stata effettivamente inviata. Questo fa in modo di permettere al dispositivo ricevente che si presuppone sia connesso ad una rete internet e abbia accesso all'orario corrente, di correggere il timestamp secondo il momento in cui la misura è stata ricevuta, andando poi a ricavare i timestamp corretti di tutte le altre misure.

Questo layer si occupa, inoltre, di inserire la misura in un messaggio che, una volta ricevuto dal gateway, viene automaticamente convertito in stringa JSON supportata da una qualsiasi IoT Platform, nel caso del progetto si fa uso di Thingsboard. Per operare, l'header `niagara_measure.h` fa uso delle seguenti classi:

- **NiagaraSender**, il dispositivo che effettua la misura e deve quindi inviarla verso un gateway selezionato. Questo oggetto contiene una lista modificabile di oggetti `NiagaraMeasure` che vengono istanziati e passati manualmente. Una volta che il sender è stato correttamente popolato con tutti gli oggetti necessari da inviare, una funzione `send(str destination)` permette l'invio di tutte le misure verso una destinazione.
- **NiagaraReceiver**, il gateway che riceve le misure fa uso di questa classe per riceverle e convertirle automaticamente in stringhe JSON supportate, che possono poi essere inoltrate via socket verso la IoTPlatform. Si usa, quindi, il metodo `receive()` che riceve la prima misura disponibile e fornisce il *Callsign* del dispositivo remoto che l'ha inviata.

# Utilizzo della libreria Niagara

- **Costruttore**  
  Il costruttore della libreria permette la creazione e gestione del logger
  `Niagara(log_handler, Niagara_LogLevel::TERMINAL);`
  - Il primo parametro è il puntatore ad una funzione di firma:
    `void funzione(const char*)`
    Questa funzione gestisce i messaggi di log prodotti dalla libreria. Astrae l'output del log dalla libreria stessa, permettendo la definizione di un output qualunque, che esso sia un file, un display o una seriale di uscita. Ció è indipendente dalla piattaforma.
  - Il secondo parametro definisce il livello di log. Qualora la funzione handler del logger stampi in output su un display di un microcontrollore a dimensioni ridotte, il logger può essere impostato con livello `Niagara_LogLevel::DISPLAY` per produrre log output piú concisi, altrimenti si può utilizzare `Niagara_LogLevel::TERMINAL` per un log completo.
  Esiste un overload di questo costruttore per non definire alcun logger, per disabilitarlo e quindi rimuovere il suo overhead:
  `Niagara();`

- **Impostazione Identificatore**
  Dopo aver chiamato il costruttore, è necessario impostare l'identificatore del dispositivo prima di poter chiamare i metodi di invio e ricezione:
  `bool set_identifier(str identifier);`
  La chiamata di questo identificatore provoca un controllo di verifica dei requisiti sull'identificatore passato. Se questo controllo va a buon fine, la funzione restituisce `true` e imposta l'identificatore nel dispositivo corrente.
  La mancata chiamata di questo metodo prima di effettuare chiamate a funzioni di invio o ricezione provoca il ritorno di `Niagara_Ret::NIAGARA_NO_IDENTIFIER` da queste.

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

## Utilizzo del layer applicativo
La libreria Niagara implementa tutto lo stack niagara di comunicazione, fino al layer di frammentazione. Il layer applicativo viene implementato separatamente da `niagara_measure`. Segue una spiegazione dell'utilizzo dei componenti del layer applicativo:

- **NiagaraMeasure** Permette la creazione di misure che possono essere trasmesse tramite il layer applicativo. Viene passato un oggetto `str` contenente la chiave, un `double` contenente il valore e un timestamp della misura che, se lasciato a `0`, fa in modo che la misura venga associata al tempo in cui viene inviata verso il gateway:

  Esempio di misura in cui viene specificato un timestamp:
  
  `NiagaraMeasure temperature("temperature", 24.5, millis());`
  
  Esempio di misura che fa uso del timestamp in cui viene inviato il messaggio:
  
  `NiagaraMeasure humidity("humidity", 50);`
- **NiagaraSender**
  Il costruttore della classe richiede il passaggio di un oggetto `Niagara` inizializzato per la comunicazione e di una quantità variabile di argomenti contenenti le misure con cui inizializzare il sender. È inoltre possibile non passare alcuna misura ed inserirle successivamente:
  `NiagaraSender(Niagara& device, NiagaraMeasure... measures);`
  - In quanto questo oggetto racchiude una lista di oggetti `NiagaraMeasure`, è inoltre possibile manipolarne il contenuto tramite ulteriori funzioni, come `add_measure` o `remove_measure`:
    
    `sender.add_measure(temperature);`
    
    `sender.remove_measure("humidity");`
    
  - Una volta riempito il contenuto del sender, è possibile inviare il messaggio tramite la funzione `send` verso la destinazione, questa ritorna qualsiasi errore riscontrato durante l'invio:
    
    `sender.send("Gateway0");`
- **NiagaraReceiver**
  Il costruttore della classe richiede, come nel sender, il passaggio di un oggetto `Niagara` inizializzato per la comunicazione:
  `NiagaraReceiver(Niagara& device);`
  - È successivamente possibile fare uso del metodo `receive` per ricevere la prima misura disponibile. Si occupa successivamente il layer applicativo di correggere i timestamp e convertire gli oggetti `NiagaraMeasure` in JSON valido che viene poi restituito dal metodo:
    
    `str json = receiver.receive(&error_code, &remote_dev);`
    
    `error_code` punta ad un valore intero contenente il codice di errore riscontrato durante la ricezione, mentre la funzione ritorna il JSON convertito.

# Codice di esempio

## Gateway
Segue un codice di esempio dell'utilizzo della libreria con il relativo layer applicativo su piattaforma Raspberry Pi 4 con modulo Waveshare LoRaHAT SX1262 in ricezione di misure:
```c++
#include <iostream>
#include <unistd.h> // Per usleep
#include "niagara/niagara.h"
#include "niagara/niagara_measure.h"

// Handler di log per la loro visualizzazione su terminale
void log_handler_terminal(const char* text) {
    printf(text);
}

// Istanziamento del dispositivo di comunicazione
//Niagara lora_device(log_handler_terminal, Niagara_LogLevel::LOG_TERMINAL); // Aggiunta di logging
Niagara lora_device;

// Funzione fittizia che simula l'inoltro all'IoT platform
void platform_send(const char* json_string) {
    std::cout << "\n========================================================" << std::endl;
    std::cout << "[SOCKET -> IOTP] Inoltro del seguente JSON:" << std::endl;
    std::cout << json_string << std::endl;
    std::cout << "========================================================\n" << std::endl;
}

int main() {
    std::cout << "--- Gateway Niagara Receiver Initialized ---" << std::endl;
    std::cout << "In ascolto di messaggi dai dispositivi IoT..." << std::endl;

    // Inizializzazione del dispositivo LoRa
    lora_device.set_identifier("Gateway0");
    // Inizializzazione del receiver
    NiagaraReceiver receiver(lora_device);

    // Ciclo infinito di ricezione ed invio
    while (true) {
        // Tenta la ricezione (il metodo chiama internamente device.recv())
        int ret;
        str remote;
        str out = receiver.receive(&ret, &remote);

        if (ret == 0) { // Ricezione avvenuta con successo (NIAGARA_OK)
            std::cout << "[LORA] Ricevuto un nuovo pacchetto valido!" << std::endl;
            // Simulazione dell'invio verso la piattaforma
            platform_send(out.c_str());
        } else {
            // Gestione del messaggio di errore ritornato dal metodo di ricezione
            std::cout << "Codice di errore ritornato dalla libreria: " << ret << std::endl;
            usleep(100000);
        }
    }

    return 0;
}
```

## Dispositivo IoT
Segue un codice di esempio dell'utilizzo della libreria su un dispositivo Heltec LoRa 32 dev board V3 basato su ESP32 con chip LoRa SX1262

```c++
#include <Arduino.h>
#include "niagara.h"
#include "niagara_measure.h"

Niagara *lora_device;

// Generazione fittizia di dati da eventuali sensori tramite numeri casuali
double randomDouble(double minVal, double maxVal) {
  double r = (double)random(0, 1000000) / 1000000.0;

  return minVal + r * (maxVal - minVal);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    Serial.println("--- IoT Device Niagara Sender Initialized ---");
    /* Creazione dell'oggetto niagara all'interno del setup() per evitare crash di FreeRTOS
     * Questo oggetto non puo' essere creato nella fase di inizializzazione del programma
     * (quindi fuori da setup() e loop()) perche' verrebbe istanziato nella fase di inizializzazione
     * e questo causerebbe l'avvio prematuro di routine asincrone sul secondo core per la ricezione
     * che portano ad un crash.
     */
    lora_device = new Niagara();
    lora_device->set_identifier("IoTDevice0");
}

void loop() {
    Serial.println("\n[INFO] Preparazione nuove misurazioni...");

    // Creazione di misure per l'invio verso il gateway
    NiagaraMeasure temp("tempIn", randomDouble(15, 25));
    NiagaraMeasure hum("humidity", randomDouble(20, 80));
    NiagaraMeasure weight("weight", randomDouble(35, 60));
    NiagaraMeasure freq("peakFreq", randomDouble(5000, 15000));

    // Inizializzazione del sender con costruttore variadico per il passaggio di tutte le misure
    NiagaraSender sender(*lora_device, temp, hum, weight, freq);

    Serial.println("[LORA] Invio dati al Gateway...");

    // Inivio dei dati verso il gateway
    int send_res = sender.send("Gateway0");

    if (send_res == 0) { // NIAGARA_OK
        Serial.println("[SUCCESS] Pacchetto LoRa inviato correttamente!");
    } else { // Errore
        Serial.print("[ERRORE] Invio fallito con codice: ");
        Serial.println(send_res);
    }

    // Invio ciclico ogni 8 secondi
    delay(8000);
}
```
