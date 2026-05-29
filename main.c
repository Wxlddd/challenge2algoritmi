#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char *argv [] ) {

char data[] = 'skreygvfbzskygfvzsuegfsukzygskyugefLçAUWfòVAUIOflie7aRPçFUHevlfuiGVlUIG'  
int length = strlen(data);
omp_set_num_threads(4);

int local_histograms[7][4] = (0);
int histogram[7] = (0);


double start_time = omp_get_wtime();

#pragma omp parallel
{
    int tid = omp_get_thread_num();
    
    #pragma omp for
    for (int i = 0; i < length; ++i) {
        int alphabet pos = (int)data[ i ] - 'a';
        if (alphabet pos >= 0 && alphabet pos < 26) {
            int bin = alphabet pos / 4;
            local_histograms [ bin ][ tid]++;
        }
    }

}

for (int b = 0; b < 7; ++b)
    for (int t = 0; t < total threads ; ++t)
        histogram [b] += local histograms [b][ t ];

double end_time = omp_get_wtime();
double time = end_time - start_time;

}

