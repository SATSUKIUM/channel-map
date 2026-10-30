# 1. "./dummy_maker.out mapdata.csv N" でmapdata.csvをN倍したダミーファイルを生成 
# 2. "./build/simple-skeleton-program ./mapdata_dummy.csv" でベンチマーク測定&ファイル出力"
# 1と2を組み合わせて2倍、4倍、8倍、16倍のダミーファイルを作成してベンチマーク測定

#!/bin/bash

ORIGINAL_FILE="./mapdata.csv"
DUMMY_FILE="./mapdata_dummy.csv"
DUMMY_MAKER="./dummy_maker.out"
SKELETON_PROGRAM="./build/simple-skeleton-program"

MULTIPLIERS=(2 4 8 16 32 48 64 80 88 96 104 112 120 128 144 160 176 192 208 224 229 234 239 244 249 253)

echo "[benchmark.sh] Running simple-skeleton-program with original file"
${SKELETON_PROGRAM} ${ORIGINAL_FILE}

# Loop through the multipliers, generate dummy files, and run the skeleton program
for N in "${MULTIPLIERS[@]}"; do
    echo "[benchmark.sh] ${N}-scaled dummy file generating"
    ${DUMMY_MAKER} ${ORIGINAL_FILE} ${N}

    echo "[benchmark.sh] Running simple-skeleton-program with ${N}-scaled dummy file"
    ${SKELETON_PROGRAM} ${DUMMY_FILE}
done

rm ./core.*
