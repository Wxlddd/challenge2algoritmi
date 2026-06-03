#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_BINS 7
#define LETTERS  26

static void gen_data(char *buf, long len) {
    for (long i = 0; i < len; i++)
        buf[i] = 'a' + rand() % LETTERS;
    buf[len] = '\0';
}

int main(int argc, char *argv[]) {

    long sizes[]   = { 10000000L, 50000000L, 100000000L, 500000000L };
    int  threads[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
    int  nsizes    = sizeof(sizes)   / sizeof(sizes[0]);
    int  nthreads_test = sizeof(threads) / sizeof(threads[0]);

    char *data = malloc(sizes[nsizes-1] + 1);
    if (!data) { fprintf(stderr, "malloc failed\n"); return 1; }

    printf("=== main.c: layout [bin][tid] ===\n");
    printf("%-12s  %-7s  %-12s  %-12s  %-12s  %-10s  %s\n",
           "len", "threads", "T.tot", "T.par", "T.red", "speedup", "efficiency");

    for (int si = 0; si < nsizes; si++) {
        long len = sizes[si];
        srand(42);
        gen_data(data, len);

        double t1 = 0.0;

        for (int ti = 0; ti < nthreads_test; ti++) {
            int nt = threads[ti];
            omp_set_num_threads(nt);

            int *lh = calloc(NUM_BINS * nt, sizeof(int));
            if (!lh) {
                fprintf(stderr, "Error: calloc failed\n");
                free(data);
                return 1;
            }
            int hist[NUM_BINS] = {0};

            double t_start = omp_get_wtime();

#pragma omp parallel
            {
                int tid = omp_get_thread_num();
#pragma omp for
                for (long i = 0; i < len; i++) {
                    int pos = data[i] - 'a';
                    if (pos >= 0 && pos < LETTERS) {
                        lh[(pos / 4) * nt + tid]++;
                    }
                }
            }

            double t_mid = omp_get_wtime();

            for (int b = 0; b < NUM_BINS; b++) {
                for (int t = 0; t < nt; t++) {
                    hist[b] += lh[b * nt + t];
                }
            }

            double t_end  = omp_get_wtime();
            double tot    = t_end - t_start;
            double par    = t_mid - t_start;
            double red    = t_end - t_mid;

            if (ti == 0) t1 = tot;
            double sp  = t1 / tot;
            double eff = sp / nt;

            char sp_str[16];
            snprintf(sp_str, sizeof(sp_str), "%.3fx", sp);

            printf("%-12ld  %-7d  %-12.6f  %-12.6f  %-12.6f  %-10s  %.4f\n",
                   len, nt, tot, par, red, sp_str, eff);

            free(lh);
        }
        printf("\n");
    }

    free(data);
    return 0;
}
