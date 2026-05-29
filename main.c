#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {

    char data[] = "skreygvfbzskygfvzsuegfsukzksytvLyeofueviuyqoywoqyetrouqyvtoqrytevbqtyvboqetyrvriuetboeirutbkeiyugbkzygrbkzyugrbfkuysgrbkuygrskuygrszkyugbrkzusygrbkzusyrzkuygrbkzsuygrbkzusyrgbzkyusgrbkzuysrlieoiewèpqpoqpqpowioppoqwepqcwouepqweqpwoqepwiieruwouryqeirueiqetuiqtwztiqibtxhmvmbcmxvowrugciurifwmivwpqpwqoieoquryeyrpqpyslhdfladkfhladhkgzvmcmzcbvklfuagaeilugfilayetyrvbqorqoreqtourvqeboyrvbqeytrqeioytrqeioyvboqeyiygskyugefLcAUWfVAUIOflie7aRPcFUHevlfuiGVlUIG";
    int  length  = strlen(data);

    int num_threads = omp_get_max_threads();

    /* local_histograms[bin][thread_id] — dimensione dinamica basata sul num. di thread */
    int (*local_histograms)[num_threads] = calloc(7, num_threads * sizeof(int));
    int histogram[7] = {0};

    double start_time = omp_get_wtime();

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
        for (int t = 0; t < num_threads; ++t)
            histogram[b] += local_histograms[b][t];

    double end_time = omp_get_wtime();
    double elapsed  = end_time - start_time;

    /* Stampa istogramma */
    printf("\n=== ISTOGRAMMA (frequenza lettere per bin) ===\n");
    for (int b = 0; b < 7; ++b) {
        int start_letter = b * 4;
        int end_letter   = start_letter + 3;
        if (end_letter > 25) end_letter = 25;
        printf("Bin %d [%c-%c]: %d\n",
               b,
               'a' + start_letter,
               'a' + end_letter,
               histogram[b]);
    }
    printf("\nTempo di esecuzione: %.6f secondi\n", elapsed);

    free(local_histograms);
    return 0;
}
