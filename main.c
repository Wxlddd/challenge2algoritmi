#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEED     42
#define NUM_BINS  7
#define LETTERS  26
#define BIN_SIZE  4

/* Genera una stringa di 'len' lettere minuscole casuali con seed fisso */
static void generate_data(char *buf, long len) {
    for (long i = 0; i < len; ++i)
        buf[i] = 'a' + (rand() % LETTERS);
    buf[len] = '\0';
}

int main(int argc, char *argv[]) {

    /* Ordini di grandezza da testare */
    static const long sizes[]   = { 10000000L, 50000000L, 100000000L, 500000000L };
    /* Numeri di thread da testare */
    static const int  threads[] = { 1, 2, 4, 8, 16, 32 , 64 , 128, 256, 512 };

    int  num_sizes   = (int)(sizeof(sizes)   / sizeof(sizes[0]));
    int  num_t_tests = (int)(sizeof(threads) / sizeof(threads[0]));
    long max_size    = sizes[num_sizes - 1];

    /* Alloca il buffer una volta sola, dimensionato sul caso peggiore */
    char *data = malloc((size_t)(max_size + 1));
    if (!data) { fprintf(stderr, "Errore malloc\n"); return 1; }

    /* Intestazione tabella */
    printf("\n%-12s  %-7s  %-12s  %-12s  %-12s  %-10s  %-12s\n",
           "Lunghezza", "Thread", "T.tot (s)", "T.par (s)", "T.red (s)", "Speedup", "Efficienza");
    printf("%-12s  %-7s  %-12s  %-12s  %-12s  %-10s  %-12s\n",
           "---------", "------", "---------", "---------", "---------", "-------", "(S/T)");

    for (int si = 0; si < num_sizes; ++si) {
        long length = sizes[si];

        /* Generazione stringa con seed fisso — FUORI dal timer */
        srand(SEED);
        generate_data(data, length);

        double baseline_elapsed = 0.0;  /* tempo con 1 thread — riferimento per lo speedup */

        for (int ti = 0; ti < num_t_tests; ++ti) {
            int nthreads = threads[ti];
            omp_set_num_threads(nthreads);

            /* local_histograms[bin][thread_id] — dimensione dinamica */
            int (*local_histograms)[nthreads] = calloc(NUM_BINS, nthreads * sizeof(int));
            if (!local_histograms) { fprintf(stderr, "Errore calloc\n"); free(data); return 1; }
            int histogram[NUM_BINS] = {0};

            /* ===== TIMER START ===== */
            double start_time = omp_get_wtime();

#pragma omp parallel
            {
                int tid = omp_get_thread_num();

#pragma omp for
                for (long i = 0; i < length; ++i) {
                    int alphabet_pos = (int)data[i] - 'a';
                    if (alphabet_pos >= 0 && alphabet_pos < LETTERS) {
                        int bin = alphabet_pos / BIN_SIZE;
                        local_histograms[bin][tid]++;
                    }
                }
            }

            double after_parallel = omp_get_wtime();   /* checkpoint dopo la regione parallela */

            for (int b = 0; b < NUM_BINS; ++b)
                for (int t = 0; t < nthreads; ++t)
                    histogram[b] += local_histograms[b][t];

            double elapsed           = omp_get_wtime() - start_time;
            double elapsed_parallel  = after_parallel  - start_time;
            double elapsed_reduction = elapsed - elapsed_parallel;
            /* ===== TIMER STOP ===== */

            if (ti == 0) baseline_elapsed = elapsed;  /* salva il tempo a 1 thread */
            double speedup    = baseline_elapsed / elapsed;
            double efficiency = speedup / nthreads;

            printf("%-12ld  %-7d  %-12.6f  %-12.6f  %-12.6f  %-10.3fx  %-12.4f\n",
                   length, nthreads, elapsed, elapsed_parallel, elapsed_reduction,
                   speedup, efficiency);

            free(local_histograms);
        }

        printf("\n"); /* riga separatrice tra taglie diverse */
    }

    free(data);
    return 0;
}
