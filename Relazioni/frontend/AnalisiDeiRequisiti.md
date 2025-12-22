# **Analisi dei Requisiti** 

# Il presente documento fornisce un'analisi completa dei requisiti per il sistema web e web app per il monitoraggio di 5 arnie, integrando le specifiche fornite con elementi tecnici essenziali per la progettazione e implementazione di un sistema IoT scalabile.

# **Panoramica del Sistema**

Il sistema di monitoraggio apiario è una piattaforma IoT che integra sensori hardware per la raccolta dati in tempo reale, un database per lo storage persistente, API REST per la comunicazione, e un'interfaccia web responsive per la visualizzazione e gestione. L'architettura supporta il monitoraggio simultaneo di 5 arnie con metriche critiche per la salute delle colonie e la produzione di miele.​

## **Visualizzazione Dati Multi-Arnia**

## Il sistema deve visualizzare simultaneamente i dati provenienti da 5 arnie distinte attraverso un'interfaccia dashboard intuitiva. Ogni arnia viene rappresentata con una card dedicata contenente gli indicatori principali (temperatura, umidità, peso, attività sonora). La dashboard implementa un layout responsivo che si adatta automaticamente a diversi dispositivi (desktop, tablet, mobile).​

## **Temperatura Interna ed Esterna**

La temperatura interna dell'arnia viene monitorata continuamente con sensori posizionati tra i telai del nido. Il range operativo tipico è 20-45°C, con la temperatura ottimale del nido che si mantiene costante intorno a 35°C anche quando la temperatura esterna varia significativamente. Il sistema registra simultaneamente la temperatura esterna per analisi comparative.​

**Produzione di Miele**

Il calcolo della produzione di miele si basa sulla variazione di peso dell'arnia. La formula applicata è:​

Miele prodotto \= Peso totale attuale \- Peso arnia vuota \- Peso stimato colonia \- Miele precedente

Considerando che una colonia media di 50.000 api pesa circa 4-6 kg (con ciascuna ape che pesa 80-100 mg), il sistema sottrae automaticamente questo valore dal peso totale per ottenere una stima accurata della produzione. Le variazioni giornaliere di peso possono oscillare tra 0.5-5 kg a seconda dell'attività di bottinatura.​

## **Umidità**

L'umidità relativa interna viene monitorata con range 0-100% RH. Il valore ottimale per una colonia sana è 50-60%. Valori troppo alti possono indicare scarsa ventilazione, mentre valori troppo bassi possono stressare la colonia.​

## **Frequenza Suono**

## L'analisi della frequenza sonora fornisce indicazioni preziose sull'attività della colonia. Il sistema monitora frequenze nel range 100-1000 Hz utilizzando un microfono con membrana acustica protetta dalla propolizzazione. L'analisi spettrale (FFT) permette di identificare pattern sonori associati alla presenza della regina, sciamature imminenti o stato di agitazione della colonia.​

**NB:** Le informazioni relative alla produzione di miele, all’umidità e alla frequenza sonora sono basate su dati medi e studi di riferimento, e possono variare in base alle condizioni ambientali e alla forza della colonia.

## **Correlazione Temperatura Interna-Esterna**

Il sistema implementa un'analisi di correlazione tra temperatura interna ed esterna dell'arnia. Questa funzionalità è utile per comprendere:​

* Efficienza termoregolazione: Le api mantengono la temperatura del nido costante indipendentemente dalle variazioni esterne​  
* Consumo energetico: Temperature esterne estreme richiedono maggior consumo di miele per termoregolazione​  
* Predizione comportamento: La correlazione aiuta a prevedere l'attività di bottinatura in base alle condizioni meteo​

Il calcolo del coefficiente di correlazione (R²) viene eseguito su finestre temporali configurabili (giornaliere, settimanali).​

**Derivata Prima (Velocità di Variazione)**

La derivata prima rappresenta la velocità di cambiamento di una metrica nel tempo:

* dT/dt: Velocità di variazione temperatura (°C/ora)  
* dW/dt: Velocità di variazione peso (kg/ora) \- indica intensità bottinatura  
* dH/dt: Velocità di variazione umidità (%/ora)

Calcolo con differenze finite: (y\[i+1\] \- y\[i\]) / (t\[i+1\] \- t\[i\])​

La derivata prima è fondamentale per il rilevamento precoce di anomalie. Ad esempio, una rapida diminuzione di peso (dW/dt negativo e elevato) può indicare sciamatura o furto.​

## **Derivata Seconda (Accelerazione del Cambiamento)**

La derivata seconda rappresenta l'accelerazione o il cambiamento della velocità di variazione:​

* d²T/dt²: Accelerazione variazione temperatura  
* d²W/dt²: Accelerazione variazione peso  
* d²H/dt²: Accelerazione variazione umidità

La derivata seconda è  utile per identificare punti di flesso e cambiamenti improvvisi nel comportamento della colonia. Un valore elevato della derivata seconda indica un cambiamento rapido nel trend, segnalando potenziali eventi critici che richiedono attenzione immediata.​

**Grafici 2D**

* *Temperatura:* Grafico a linee per visualizzare trend temporali. Supporta dual-axis per mostrare simultaneamente temperatura interna ed esterna​  
* *Umidità:* Combinazione di grafico a linee (trend) e gauge widget (valore corrente). Zone colorate (verde=ottimale, giallo=attenzione, rosso=critico) per immediata comprensione​  
* *Peso/Miele:* Grafico a linee cumulativo per produzione totale, grafico a barre per incrementi giornalieri​  
* *Suono:* Spettrogramma per analisi frequenze nel tempo, combinato con grafico linee per frequenza dominante​  
* *Correlazione:* Scatter plot per visualizzare relazione temperatura interna-esterna con regressione lineare​

## **Grafici 3D**

I grafici 3D sono particolarmente utili per visualizzare relazioni complesse tra più variabili nel tempo:​

* *Superficie 3D temperatura:* Asse X \= tempo, Asse Y \= posizione sensore, Asse Z \= temperatura. Mostra distribuzione spaziale-temporale​  
* *Grafico 3D peso-temperatura-umidità:* Visualizza correlazioni multivariate​  
* *Heatmap 3D:* Rappresentazione densità dati nel volume osservazione​

**Gestione Utenti e Permessi**

Il sistema implementa un robusto modello con tre livelli di accesso:​

1. Admin (Amministratore)  
* Accesso completo a tutte le funzionalità​  
* Gestione utenti (creazione, modifica, eliminazione)​  
* Configurazione sistema e regole allarmi​  
* Accesso a tutti i dati storici e log attività​  
* Permessi: READ, WRITE, DELETE, MANAGE\_USERS​  
2. Operator (Operatore)  
* Visualizzazione dati real-time e storici​  
* Gestione allarmi (conferma, risoluzione)​  
* Modifica stato arnie (manutenzione, attiva)​  
* Esportazione dati​  
* Permessi: READ, WRITE (limitato ad allarmi)​  
3. Viewer (Visualizzatore)  
* Accesso read-only alla dashboard​  
* Visualizzazione grafici e storico​  
* Nessuna capacità di modifica​  
* Permessi: READ only​

**Sistema di generazione automatica di errori** 

Il sistema di gestione allarmi è progettato per garantire risposta tempestiva a condizioni critiche.​ Gli allarmi vengono generati automaticamente quando i valori dei sensori superano soglie predefinite:​

* Temperatura critica: T \< 20°C o T \> 40°C​  
* Peso anomalo: Perdita \> 5 kg/giorno (possibile sciamatura o furto)​  
* Umidità anomala: H \< 30% o H \> 80%​  
* Inattività sonora: Assenza segnale per periodo prolungato

---

Basaglia Noemi \- Bego Francesco \- Zara Luigi

da aggiungere:  
tipo di grafico migliore per ogni dato (grafici 3d)  
gestione utente (permessi/autorizzazioni)  
temperatura esterna (rapporto con interna)  
gestione risolvere allarmi (comunicazione al database)  
far vedere tutti i dati necessari e rapporti (derivate prime, seconde)

Ciao Alfredo, ti inoltro quello che volevo scrivere sulla rendicontazione del progetto se fosse partito: ovviamente questo è calcolato per un aria di piccole dimensioni. Mi informo con matteo e ti dico le specifiche delle arnie che ha

# **Peso delle api e dell’arnia**

Le api rappresentano una frazione relativamente piccola ma essenziale del peso totale di un’arnia. Una colonia media di circa **50.000 individui** contribuisce con **0,5–1 kg** al peso complessivo, mentre il peso totale della colonia si aggira attorno a **5–6 kg**. Questo valore è modesto rispetto al peso dell’intera arnia, che include anche la struttura, i telai, il miele e le altre scorte.

---

**Dettagli sul peso delle api**

* **Peso della singola ape**: circa **100 mg** (0,1 g).  
* **Numero di api in una colonia**: in media **50.000**, ma può essere superiore in periodi di piena attività.  
* **Peso totale della colonia**: moltiplicando il peso medio di una singola ape per il numero di api, si ottiene un valore di circa **5–6 kg**, equivalente a un quantitativo simile di miele.

---

**Componenti del peso complessivo di un’arnia**

* **Struttura dell’arnia**: il materiale (legno, polistirene, ecc.) e le dimensioni influiscono notevolmente sul peso.  
* **Telai**: realizzati in legno o plastica, aggiungono ulteriore massa all’insieme.  
* **Miele**: rappresenta una delle voci principali, poiché le api immagazzinano scorte per il sostentamento e per l’inverno.  
* **Altre risorse**: polline, pappa reale e altri nutrienti raccolti dalle api contribuiscono al peso complessivo.