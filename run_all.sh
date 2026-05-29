#!/bin/bash

OUT="risultati.txt"
CC="gcc"
FLAGS="-O3 -fopenmp"

echo "Compiling..."
$CC $FLAGS main.c  -o v1 || { echo "Compile error: main.c";  exit 1; }
$CC $FLAGS main2.c -o v2 || { echo "Compile error: main2.c"; exit 1; }
$CC $FLAGS main3.c -o v3 || { echo "Compile error: main3.c"; exit 1; }
echo "Done."

{
    echo "========================================"
    echo "  Benchmark istogramma OpenMP"
    echo "  $(date)"
    echo "========================================"
    echo ""

    echo "--- v1: main.c  [bin][tid] ---"
    ./v1
    echo ""

    echo "--- v2: main2.c [tid][bin] ---"
    ./v2
    echo ""

    echo "--- v3: main3.c [tid][PADDED_BINS] ---"
    ./v3
    echo ""

} | tee "$OUT"

echo "Results saved to $OUT"
