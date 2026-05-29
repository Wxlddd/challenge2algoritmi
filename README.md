# Challenge 2 — Parallel Histogram with OpenMP

Calcolo di un istogramma di frequenza su lettere dell'alfabeto usando OpenMP.
Tre implementazioni con layout di memoria diversi per analizzare l'impatto del
false sharing sulle performance.

---

## File

| File | Descrizione |
|---|---|
| `main.c` | Layout `[bin][tid]` — false sharing (baseline) |
| `main2.c` | Layout `[tid][bin]` — no false sharing |
| `main3.c` | Layout `[tid][PADDED_BINS=16]` — padding a 64 byte per cache line |
| `run_all.sh` | Compila ed esegue le tre versioni, salva output in `risultati.txt` |
| `domanda1.md` | Risposta alla domanda 1 del challenge |
| `risultati.txt` | Output del benchmark (generato da `run_all.sh`) |

---

## Compilazione e run

```bash
chmod +x run_all.sh
./run_all.sh
```

Oppure manualmente:

```bash
gcc -O3 -fopenmp main.c  -o v1
gcc -O3 -fopenmp main2.c -o v2
gcc -O3 -fopenmp main3.c -o v3
./v1
./v2
./v3
```

---

## Cosa fa il programma

Genera una stringa casuale di lettere minuscole (seed fisso = 42) e conta
quante lettere cadono in ciascuno dei 7 bin (a-d, e-h, i-l, m-p, q-t, u-x, y-z).

Il calcolo viene parallelizzato con `#pragma omp parallel for`. Ogni thread
mantiene un array locale di contatori per evitare race condition sulla scrittura;
alla fine i contatori locali vengono sommati nell'istogramma globale.

Il benchmark varia:
- **Lunghezza input**: 10M, 50M, 100M, 500M caratteri
- **Numero di thread**: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096

Per ogni combinazione viene stampato: tempo totale, tempo della fase parallela,
tempo della riduzione, speedup e efficienza parallela (speedup / n_thread).

---

## Le tre versioni a confronto

### `main.c` — layout `[bin][tid]`

```c
int (*lh)[nt] = calloc(NUM_BINS, nt * sizeof(int));
lh[bin][tid]++;
```

Thread diversi scrivono sulla stessa riga della matrice → **false sharing**:
ogni scrittura invalida la cache line degli altri thread. Lo speedup è
praticamente nullo o negativo già a 2-4 thread.

### `main2.c` — layout `[tid][bin]`

```c
int (*lh)[NUM_BINS] = calloc(nt, NUM_BINS * sizeof(int));
lh[tid][bin]++;
```

Layout trasposto: ogni thread scrive sulla propria riga. Il false sharing è
ridotto ma non eliminato del tutto perché 7 int = 28 byte < 64 byte (cache line),
quindi righe adiacenti possono ancora condividerla.

### `main3.c` — layout `[tid][PADDED_BINS=16]`

```c
int **lh = malloc(nt * sizeof(int *));
lh[t] = calloc(PADDED_BINS, sizeof(int));  // 16 * 4 = 64 byte = 1 cache line
lh[tid][bin]++;
```

Ogni thread ottiene esattamente 64 byte di spazio (una cache line intera).
Le allocazioni separate garantiscono che nessun altro thread condivida mai
quella cache line → **zero false sharing**.

---

## Risultati chiave (500M caratteri)

| Versione | Speedup picco | Thread al picco |
|---|---|---|
| `main.c`  (false sharing)      | ~2.4x | 256 |
| `main2.c` (no false sharing)   | ~3.6x | 128 |
| `main3.c` (padding 64B)        | ~3.9x | 64  |

Lo speedup si satura intorno a 4-5x indipendentemente dal numero di thread
perché il workload è **memory-bound**: il collo di bottiglia è la banda della
RAM (~12-15 GB/s), non la capacità di calcolo dei core. Il comportamento
non-monotono a certi thread count è dovuto ad effetti **NUMA** (accesso remoto
alla memoria tra socket).

---

## Requisiti

- GCC con supporto OpenMP (`-fopenmp`)
- Linux / WSL (lo script usa bash)