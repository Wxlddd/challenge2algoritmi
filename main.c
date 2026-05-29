#pragma omp parallel
{
    int tid = omp get thread num ();
    
    #pragma omp for
    for (int i = 0; i < length; ++i) {
        int alphabet pos = (int)data[ i ] - 'a';
        if (alphabet pos >= 0 && alphabet pos < 26) {
            int bin = alphabet pos / 4;
            local histograms [ bin ][ tid]++;
        }
    }

}
for (int b = 0; b < 7; ++b)
    for (int t = 0; t < total threads ; ++t)
        histogram [b] += local histograms [b][ t ];