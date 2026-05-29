# Domanda 1 — Completamento del codice

Il codice scheletro era già strutturalmente corretto. Mancavano due cose:
l'inizializzazione delle variabili e la misurazione del tempo.

---

## 1. Inizializzazione delle variabili

Nel codice scheletro erano presenti variabili usate ma non dichiarate né
inizializzate. Queste sono le aggiunte necessarie:

### `data` — la stringa di input
```c
char data[] = "skreygvfbzskygfvzsuegfsukzygskyugef...";
int length = strlen(data);
```
`data` contiene la sequenza di caratteri da analizzare; `length` ne indica
la lunghezza, usata come bound nel ciclo parallelo.

### `total_threads` — numero di thread
```c
int total_threads = omp_get_max_threads();
```
Restituisce il numero massimo di thread che OpenMP utilizzerà nella regione
parallela. Serve come bound nel ciclo di riduzione finale.

### `local_histograms` e `histogram` — i contatori
```c
int local_histograms[7][total_threads]; // un contatore per bin per thread
int histogram[7] = {0};                // istogramma globale finale
```
Senza l'inizializzazione a `{0}`, le variabili locali in C contengono valori
indefiniti (garbage), rendendo i risultati dell'istogramma completamente errati.
`local_histograms` viene azzerato implicitamente se dichiarato come VLA locale,
o esplicitamente con `memset`/`calloc` se allocato dinamicamente.

---

## 2. Misurazione del tempo di esecuzione

Mancava completamente la misurazione. Abbiamo usato `omp_get_wtime()`,
la funzione wall-clock ad alta risoluzione fornita da OpenMP.

Il timer viene avviato **subito prima** della regione parallela e fermato
**dopo** la riduzione, in modo da includere entrambe le fasi:

```c
double start_time = omp_get_wtime();   // START

#pragma omp parallel
{
    int tid = omp_get_thread_num();
    #pragma omp for
    for (int i = 0; i < length; ++i) {
        int alphabet_pos = (int)data[i] - 'a';
        if (alphabet_pos >= 0 && alphabet_pos < 26) {
            int bin = alphabet_pos / 4;
            local_histograms[bin][tid]++;
        }
    }
}

for (int b = 0; b < 7; ++b)
    for (int t = 0; t < total_threads; ++t)
        histogram[b] += local_histograms[b][t];

double elapsed = omp_get_wtime() - start_time;   // STOP

printf("Tempo di esecuzione: %.6f secondi\n", elapsed);
```

---

## Ottimizzazioni successive

Dopo aver completato il codice base, abbiamo apportato alcune ottimizzazioni:

- **Dimensione dinamica di `local_histograms`**: invece di allocare `[7][4]`
  con il numero di thread hardcoded, si usa `omp_get_max_threads()` e `calloc`
  (che azzera automaticamente):
  ```c
  int num_threads = omp_get_max_threads();
  int (*local_histograms)[num_threads] = calloc(NUM_BINS, num_threads * sizeof(int));
  ```

- **Benchmark multi-taglia e multi-thread**: loop su dimensioni dell'input
  (10 M, 50 M, 100 M, 500 M caratteri) e su numeri di thread crescenti,
  con stampa di speedup ed efficienza parallela per ogni combinazione.

- **Eliminazione del false sharing** (`main2.c`, `main3.c`): layout trasposto
  della matrice `[tid][bin]` e padding a 64 byte per cache line, per evitare
  che thread diversi invalidino reciprocamente le stesse cache line.
