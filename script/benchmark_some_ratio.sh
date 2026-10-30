#!/bin/bash

EXCUTABLE="./build/simple-skeleton-program"
INPUT_FILE="./mapdata.csv"
RATIO=(1e-6 1e-5 1e-4 1e-3 1e-2 1e-1)
for r in "${RATIO[@]}"; do
    echo "Running with ratio: $r"
    $EXCUTABLE $INPUT_FILE $r
done
echo "Finished all runs."

rm ./core.*