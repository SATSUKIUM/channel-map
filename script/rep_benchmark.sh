#!/bin/bash

for i in {1..10}; do
    echo "Run $i"
    ./benchmark.sh
done

rm ./core.*