# Challenge 2 — Parallel Histogram con OpenMP

Questo progetto contiene tre implementazioni in linguaggio C per il calcolo parallelo di un istogramma di frequenza su lettere dell'alfabeto utilizzando **OpenMP**.
Le tre versioni analizzano l'impatto del layout di memoria e del **false sharing** sulle prestazioni finali di scalabilità.

---

## Struttura della Repository

| File / Cartella | Descrizione |
| :--- | :--- |
| `src/main.c` | **Versione Base (v1)** — Layout `[bin][tid]`. Soffre gravemente di false sharing. Compatibile con tutti i compilatori C standard (non fa uso di VLA). |
| `src/main2.c` | **Versione Trasposta (v2)** — Layout `[tid][bin]`. Riduce il false sharing disponendo i dati in modo consecutivo per thread. |
| `src/main3.c` | **Versione Padded (v3)** — Layout `[tid][PADDED_BINS=16]`. Risolve completamente il false sharing allineando ogni riga della matrice locale a 64 byte (dimensione tipica di una cache line) usando una singola allocazione bidimensionale padded. |
| `run_all.sh` | Script bash che automatizza compilazione ed esecuzione dei benchmark, salvando i report in `risultati.txt`. |
| `doc/domanda1.md` | Risposte testuali ed analisi teorica delle parti omesse nello scheletro del challenge. |
| `report/main.tex` | Sorgente LaTeX per la generazione della relazione finale (con grafici e tabelle). |

---

## Guida alla Riproducibilità dei Risultati

Per riprodurre fedelmente i benchmark di scaling e i tempi presentati nella relazione, **è fortemente consigliato l'utilizzo di Linux tramite WSL** (Windows Subsystem for Linux), in quanto lo scheduler dei thread e le performance di OpenMP sotto Windows nativo variano in modo significativo rispetto all'ambiente Linux di riferimento.

Segui le istruzioni dettagliate qui sotto, pensate per chi non ha familiarità con il terminale.

### 1. Installazione e Avvio di WSL (se non installato)

1. Apri la barra di ricerca di Windows e digita **PowerShell**.
2. Fai clic destro su PowerShell e seleziona **Esegui come amministratore**.
3. Nella finestra di PowerShell che si apre, digita il comando seguente e premi Invio:
   ```powershell
   wsl --install
   ```
4. Attendi che Windows scarichi e installi Ubuntu. Al termine del processo, **riavvia il computer** se richiesto.
5. Dopo il riavvio, si aprirà automaticamente una console nera (il terminale Linux di Ubuntu) che ti chiederà di inserire un **Username** e una **Password** a tua scelta. Scegli credenziali semplici e annotale (la password non mostrerà caratteri mentre la digiti, è normale).

### 2. Installazione degli strumenti di sviluppo (GCC e OpenMP)

Nel terminale di Ubuntu (WSL) aperto, digita i seguenti comandi uno per uno premendo Invio alla fine di ciascuno:

1. Aggiorna l'elenco dei pacchetti disponibili:
   ```bash
   sudo apt update
   ```
   *(Nota: ti verrà chiesta la password che hai creato al punto precedente. Digitala e premi Invio).*

2. Installa il compilatore GCC e le librerie necessarie (inclusa la libreria OpenMP):
   ```bash
   sudo apt install -y build-essential
   ```

### 3. Posizionarsi nella directory del progetto

Da WSL è possibile accedere direttamente ai file di Windows. Se hai scaricato la repository in una cartella di Windows (ad esempio in `Documents\VSCodeProjects\challenge2`), puoi posizionarti lì dentro digitando nel terminale WSL:

```bash
cd "/mnt/c/Users/user/Documents/VSCodeProjects/challenge2"
```

Per verificare di essere nella cartella giusta, digita `ls` e premi Invio: dovresti vedere la lista dei file del progetto (`src`, `report`, `run_all.sh`, ecc.).

### 4. Esecuzione del Benchmark

1. Concedi i permessi di esecuzione allo script di benchmark:
   ```bash
   chmod +x run_all.sh
   ```
2. Esegui lo script:
   ```bash
   ./run_all.sh
   ```
3. Lo script compilerà i tre file sorgente (`src/main.c`, `src/main2.c`, `src/main3.c`) con ottimizzazioni aggressive (`-O3`) e il supporto OpenMP (`-fopenmp`), dopodiché avvierà i test per ogni dimensione del dataset ed ogni conteggio di thread.
4. I risultati verranno stampati a video in tempo reale e salvati automaticamente nel file **`risultati.txt`**.

