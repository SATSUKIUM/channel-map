#!/bin/bash
numreps=100
echo "Running simple-skeleton-program $numreps times..."
num_dummy=$1
command="./build/simple-skeleton-program ./mapdata.csv"
if [ "$num_dummy" -eq 0 ]; then
    echo "Running with original file..."
else
    command1="./dummy_maker.out ./mapdata.csv $num_dummy"
    echo "Generating dummy file with multiplier $num_dummy..."
    $command1
    command="./build/simple-skeleton-program ./mapdata_dummy.csv"
fi
for i in $(seq 1 $numreps); do
    echo "Run $i"
    $command
done

rm ./core.*