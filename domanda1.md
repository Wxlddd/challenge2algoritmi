# Domanda 1 — Completamento del codice e Scelte di Implementazione

La versione originaria presentava alcune omissioni e criticità strutturali, che sono state risolte con implementazioni conformi agli standard industriali del linguaggio C per garantire massima portabilità e riproducibilità.

---

## 1. Inizializzazione delle variabili

Per rendere il programma compilabile ed eseguibile, sono stati dichiarati e inizializzati correttamente i seguenti elementi:

### Buffer dei dati (`data` e `length`)
Il buffer di input `data` viene allocato dinamicamente in base alle dimensioni del dataset prescelto (fino a 500 milioni di caratteri). Per garantire che l'input sia coerente e confrontabile tra le varie versioni, la generazione della stringa casuale viene guidata da un seed fisso (`srand(42)`).

### Riduzione seriale finale e conteggio thread
Il numero di thread da testare viene configurato a ogni passo tramite `omp_set_num_threads(nt)`. Il numero massimo di thread istanziati serve poi come limite superiore per il ciclo di riduzione finale.

### Allocazione dei contatori locali
Per evitare l'uso di Variable-Length Arrays (VLA) a livello di puntatore (`int (*lh)[nt]`), che non sono supportati in modo nativo su tutti i compilatori (come MSVC su Windows) e sono opzionali a partire dallo standard C11, si è preferito utilizzare un array lineare monodimensionale allocato via `calloc`:
```c
int *lh = calloc(NUM_BINS * nt, sizeof(int));
```
Questa struttura viene azzerata automaticamente da `calloc` ed è indicizzata tramite calcolo dell'offset `lh[bin * nt + tid]`. L'istogramma globale finale `hist` viene azzerato esplicitamente prima del calcolo per evitare valori indefiniti presenti in memoria (garbage value).

---

## 2. Misurazione del tempo di esecuzione

La misurazione del tempo viene effettuata tramite la funzione ad alta precisione `omp_get_wtime()`. 
Per isolare e misurare con precisione il comportamento parallelo ed evidenziare l'impatto del false sharing, il codice campiona tre momenti distinti:

- `t_start`: Subito prima dell'avvio del costrutto `#pragma omp parallel`.
- `t_mid`: Immediatamente al termine del costrutto parallelo, prima della riduzione seriale.
- `t_end`: Al termine del calcolo della riduzione seriale finale.

Da questi checkpoint si ricavano tre intervalli:
- `T.tot = t_end - t_start`: Tempo complessivo del benchmark.
- `T.par = t_mid - t_start`: Tempo speso esclusivamente nella fase parallela di conteggio.
- `T.red = t_end - t_mid`: Tempo speso nella fase seriale di riduzione finale.

L'analisi empirica dimostra che `T.red` (riduzione seriale) è dell'ordine dei microsecondi ed è del tutto trascurabile rispetto a `T.par` su tutte le taglie e thread count testati.

---

## 3. Ottimizzazioni e Struttura della Memoria

### Versione Base (`main.c`)
I thread condividono la stessa riga della matrice in quanto indicizzati come `lh[bin * nt + tid]`. Poiché le locazioni contigue in memoria sono modificate da thread diversi, si verifica il fenomeno del **false sharing** che causa l'invalidazione continua delle linee di cache e degrada le performance.

### Versione Trasposta (`main2.c`)
Ogni thread scrive in una riga dedicata contigua di 7 interi (`7 * 4 = 28` byte) tramite il layout `lh[tid][bin]`. Questo riduce il false sharing ma non lo elimina del tutto, in quanto la dimensione della riga è inferiore a una cache line standard (64 byte), permettendo a righe di thread diversi di condividere la stessa linea fisica.

### Versione Padded (`main3.c`)
Ogni thread scrive su una riga allineata a 64 byte tramite il layout `lh[tid][bin]` con `PADDED_BINS = 16` (`16 * 4 = 64` byte):
```c
int (*lh)[PADDED_BINS] = calloc(nt, PADDED_BINS * sizeof(int));
```
Con un'unica chiamata pulita ed efficiente a `calloc`, ciascun thread ottiene una cache line intera dedicata, garantendo la totale assenza di false sharing senza overhead di allocazione.
