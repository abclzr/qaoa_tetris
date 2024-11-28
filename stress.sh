#!/usr/bin/env bash
for density in 3 5; do
    for ((size = 16; size <= 17; size++)); do
        echo -n "${size},${density},"
        timeout 1h ./build/subiso "Benchmarks/others/${size}_${density}_0.txt" ${size}
        echo
    done
done