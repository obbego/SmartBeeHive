# Analisi dei requisiti
Questo documento vuole essere una versione aggiornata dell’analisi dei requisiti del progetto già avviato.  
Lo scopo è quello di ristabilire i nuovi requisiti, in modo da avere punti precisi sui quali sviluppare il lavoro dei prossimi mesi.

## Descrizione del progetto
Il progetto viene realizzato in collaborazione con l’ITAS di Rovigo.  
L’obiettivo è raccogliere una serie di dati dalle arnie con una certa scansione oraria e successivamente visualizzarli tramite un’interfaccia grafica.

Tali dati potranno poi essere utilizzati per determinare quando raccogliere il miele o per segnalare eventuali informazioni riguardanti lo stato delle singole arnie.

### Dati da misurare
I dati da rilevare all’interno di ogni arnia sono i seguenti:
- **TEMPERATURA**
- **UMIDITÀ**
- **RUMORE**, per misurare l’attività delle api

Tali dati dovranno essere rilevati seguendo una specifica scansione oraria, che potrà essere stabilita in seguito.

### Luogo
![Posizione alveare](img/distanza_alveare.png)  
*Posizione dell’alveare all’interno dell’ITA*

Come è possibile vedere, il luogo in cui l’alveare sarà collocato è un campo aperto, che potrebbe presentare ostacoli come vento o passaggio di persone durante il suo periodo di funzionamento. La distanza dal punto più vicino della scuola è di circa 100 m.  
**Nel punto verranno installate n. 5 arnie**, quindi dovranno essere presenti 5 apparati di misurazione che comunicheranno tutti i dati raccolti.

## Indicazioni del cliente
Di seguito vengono riportate le indicazioni del cliente in merito alle caratteristiche delle api da tenere in considerazione.

### Peso delle api e dell’arnia
Le api rappresentano una frazione relativamente piccola ma essenziale del peso totale di un’arnia. Una colonia media di circa **50.000 individui** contribuisce con **0,5–1 kg** al peso complessivo, mentre il peso totale della colonia si aggira attorno a **5–6 kg**. Questo valore è modesto rispetto al peso dell’intera arnia, che include anche la struttura, i telai, il miele e le altre scorte.

### Dettagli sul peso delle api
- **Peso della singola ape**: circa **100 mg** (0,1 g).
- **Numero di api in una colonia**: in media **50.000**, ma può essere superiore nei periodi di piena attività.
- **Peso totale della colonia**: moltiplicando il peso medio di una singola ape per il numero di api si ottiene un valore di circa **5–6 kg**, equivalente a una quantità simile di miele.

### Componenti del peso complessivo di un’arnia
- **Struttura dell’arnia**: il materiale (legno, polistirene, ecc.) e le dimensioni influiscono notevolmente sul peso.
- **Telai**: realizzati in legno o plastica, aggiungono ulteriore massa all’insieme.
- **Miele**: rappresenta una delle voci principali, poiché le api immagazzinano scorte per il sostentamento e per l’inverno.
- **Altre risorse**: polline, pappa reale e altri nutrienti raccolti dalle api contribuiscono al peso complessivo.

## Scelte effettuate
Durante i primi mesi sono già state definite alcune scelte per la continuazione del progetto. Qui verranno solo esposte; di seguito verranno analizzate le [problematiche](#problematiche-riscontrate).

### Schema di collegamento
![Schema alveare](img/schema_rete.png)  
*Esempio di collegamento tra i vari componenti di rete*

**NOTA BENE**  
*Alcuni collegamenti dello schema, come i componenti utilizzati in Packet Tracer, sono a scopo esemplificativo. La nomenclatura, inoltre, non corrisponde perfettamente ai dispositivi e alla tecnologia attualmente adottata. Seguire le indicazioni seguenti per conoscere gli ultimi dispositivi e la tecnologia in uso.*

La rete adottata per la comunicazione dei dati delle singole arnie è composta dai seguenti elementi:
1. *SCHEDA ALVEARE*, presente per ogni arnia, alla quale sono collegati i sensori. Dalla scheda partono i dati relativi ai segnali rilevati.  
2. *GATEWAY ALVEARI*, che raccoglie tutti i dati degli alveari e li trasmette all’antenna più vicina.  
3. *SISTEMA DI RICEZIONE*, costituito da un’antenna posizionata all’esterno dell’edificio più vicino all’agrario. Comunicherà al server i dati raccolti.  
4. *SERVER*, che raccoglie i dati che verranno successivamente elaborati.

### Schede alveare
La scheda pensata per la rilevazione dei dati dell’alveare è l’**ESP32**, reperita dalla scuola. Tale dispositivo dispone di un chip **SX1262**, in grado di instaurare una comunicazione wireless come il **LoRaWAN**.

### Dispositivi del sistema sensoristico alveare
Di seguito è riportato l’elenco dei sensori scelti per la misurazione dei vari dati nelle singole arnie:
- [**DHT22**](https://amzn.eu/d/d1KN32y): sensore in grado di misurare temperatura e umidità. Facilmente programmabile con Arduino e resistente a escursioni termiche elevate.
- [**Cella di carico**](https://www.tinkerforge.com/en/shop/load-cell-100kg-czl601.html): verrà posizionata all’interno dell’arnia per misurare il peso dell’alveare. La misurazione funziona anche con l’alveare inclinato.
- [**Microfono omnidirezionale**](https://www.amazon.it/AHELECTRO-microfono-sensibilit%C3%A0-rilevamento-compatibile/dp/B0FJX3KJB1): compatibile con Arduino, garantisce una facile configurazione.

Per l’alimentazione del sistema si è pensato a:
- [**Batteria AGM**](https://www.leroymerlin.it/prodotti/green-cell-agm04-batteria-ups-acido-piombo-vrla-12-v-7-ah-91021332.html): batteria che resiste a inclinazioni e vibrazioni, offrendo maggiore autonomia di utilizzo.  
- [**Pannello solare**](https://www.amazon.it/DEWIN-Policristallino-Caricabatterie-Passerella-Connettore/dp/B0CLGFPZ48): prodotto di alta efficienza, leggero e portatile.

### Gateway alveare
Per il gateway delle arnie si è pensato a un **Raspberry Pi**. Durante i primi mesi è stato già configurato con **Raspbian** come sistema operativo. A tale dispositivo verrà collegato un modulo *LoRa HAT* per la trasmissione dei dati.  

Un Raspberry verrà utilizzato anche dopo la ricezione dei dati dall’antenna collegata all’ITA, in modo da poter inviare i dati a un server.

### Configurazione del server
Per la configurazione del server si è deciso di utilizzare **ThingsBoard**, che implementa un database per la raccolta dati e un’interfaccia grafica. Grazie al sistema di account e alla personalizzazione, consente di mostrare, raccogliere e gestire la visualizzazione dei dati anche lato cliente.

## Problematiche riscontrate
### Analisi dei problemi
Durante lo sviluppo del progetto estivo e le riunioni svolte al rientro a scuola sono emerse varie problematiche, in particolare:
1. **Assenza di Wi-Fi** – Non è possibile utilizzare la tecnologia Wi-Fi per la comunicazione delle informazioni. Le api, infatti, possono risentire delle frequenze emesse da tale comunicazione. Anche la proposta di collegare in modo cablato i vari componenti del sistema dell’alveare è stata finora scartata.  
2. **Impossibilità di utilizzo del LoRaWAN** – Alternativa al Wi-Fi era il protocollo LoRaWAN, che sfrutta frequenze più basse e opera al livello 3 della pila ISO-OSI. Tuttavia, per implementare il LoRaWAN, è necessario che uno dei due dispositivi di comunicazione sia impostato in modalità server, cosa non possibile con i chip SX1262 in uso. L’unica soluzione finora individuata è l’utilizzo del solo protocollo **LoRa**, che però è soggetto a collisioni e risulta unidirezionale.  
3. **Trasmissione dei dati** – Le api potrebbero risentire di una comunicazione sempre attiva tra i dispositivi. Per questo è stato richiesto di trasmettere i dati tramite onde radio solo tre volte al giorno. Inoltre, occorre considerare i dati corrotti che potrebbero essere trasmessi a causa di interferenze ambientali.  
4. **Alimentazione dell’arnia** – Il sistema di sensori collegato a ogni arnia dovrà essere alimentato da una batteria collegata a un pannello solare. Tuttavia, la quantità di energia raccolta potrebbe non essere sufficiente per mantenere sempre attivo il sistema, soprattutto in caso di maltempo o nella stagione invernale.  
5. **Condizioni ambientali** – Oltre all’alimentazione, il maltempo e le condizioni ambientali possono influire sul sistema di sensori. Occorre quindi scegliere sensori in grado di resistere a tali condizioni e progettare una scatola che protegga l’apparato.  
6. **Rilevazione del rumore** – Il rumore rilevato all’interno dell’alveare indica lo stato di attività delle api. Tuttavia, tali dati richiedono campionamento e raccolta per un certo intervallo di tempo, con conseguente consumo di energia. Inoltre, bisogna verificare che il rumore provenga effettivamente dalle api e non da fattori esterni.  
7. **Programmazione dell’antenna** – Occorre ancora individuare un’antenna da esterno adatta alla comunicazione con le arnie.  
8. **Scansione oraria** – Le misurazioni sono state richieste in base alla luminosità, ma occorre adattare questo criterio alle stagioni e alle variazioni momentanee di luce.

### Possibili soluzioni
Sono state avanzate alcune possibili soluzioni, tuttora da sviluppare:
1. **Utilizzo del LoRa**, nonostante i problemi precedentemente elencati. L’idea è quella di costruire un protocollo semplificato al di sopra, in modo da garantire una comunicazione senza collisioni.  
2. **Ripetizione nell’invio dei dati** – Poiché il numero di interazioni tra i dispositivi è basso, una soluzione potrebbe essere la loro memorizzazione. In questo modo i dati verranno raccolti e trasmessi solo nei momenti consentiti. Ogni dato potrà inoltre essere inviato più volte per ridurre eventuali errori.  
3. **Sviluppo della scatola dei sensori**, che verrà posizionata all’esterno dell’arnia. Questa soluzione garantisce un buon grado di protezione e riduce al minimo l’interferenza con le api.  
4. **Scheda dedicata al rilevamento del rumore** ***(PROPOSTA DA VERIFICARE)*** – Questa scheda si attiverà solo al superamento di una soglia di segnale. Da quel momento effettuerà il campionamento del segnale per rilevare rumore e frequenza, consentendo al resto del sistema di operare indipendentemente. Al termine, i dati verranno comunicati alla scheda principale.  
5. **Utilizzo di una fotoresistenza** per determinare il momento in cui effettuare la misurazione. Occorre tuttavia prestare attenzione a possibili variazioni momentanee di luce (illuminazione artificiale, ombre, ecc.).

## Requisiti attuali
Alla luce della situazione attuale e dei relativi problemi, ecco i requisiti da soddisfare durante la continuazione del progetto:

### Sistema sensoristico
1. Completare la progettazione della scatola contenente la scheda con i collegamenti ai sensori.  
2. Testare il sensore DHT22.  
3. Testare la cella di carico.  
4. Testare il microfono, con le relative modalità di accensione, spegnimento e campionamento.  
5. Verificare la possibilità di utilizzo della fotoresistenza con tutti i controlli del caso.  
6. Scrivere il codice della scheda per la rilevazione dei dati.  
7. Gestire l’invio ripetitivo dei dati e il loro salvataggio temporaneo.  

*Per i test è stato proposto di sfruttare i kit di Arduino con breadboard e scheda ESP32, in modo da controllare la piena funzionalità dei sensori.*

### Gateway alveari
1. Progettare l’esatto scambio di informazioni con tutti i possibili messaggi tra alveari e server.  
2. Implementare il codice di comunicazione all’interno del gateway.

### Protocollo di comunicazione
1. Completare la progettazione del protocollo da aggiungere al LoRa.  
2. Scrivere un primo codice di prova e testarne il funzionamento.  
3. Implementare tale protocollo nelle schede degli alveari e nei Raspberry.

### Ricevitore
1. Verificare la validità dell’antenna attuale per la ricezione dei dati.  
2. Testare una possibile ricezione dei dati.  
3. Testare l’invio dei dati verso il server.  
4. Scrivere il codice definitivo.

### Server
1. Configurare il server con ThingsBoard.  
2. Configurare il database.  
3. Configurare la gestione degli account.  
4. Configurare un’interfaccia grafica di prova per l’output dei risultati.

**N.B.**  
*Per l’output dei dati si è pensato di utilizzare un’interfaccia grafica dedicata e non quella di default del server ThingsBoard. Tale interfaccia ha lo scopo di essere più accessibile e facilmente collegabile al sito della scuola e ad altri progetti in corso.*

## Link utili
- [Documentazione ThingsBoard](https://thingsboard.io/docs/)
- [Documentazione LoRa](https://lora.readthedocs.io/en/latest/)
- [Relazione estiva progetto](https://docs.google.com/document/d/1Dwv33JR1j2aQpMVEqqzAZ313RDTC_d7JLzsLLFGhavA/edit?usp=sharing)