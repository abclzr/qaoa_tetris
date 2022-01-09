#!/usr/bin/env bash

for ((size = 8; size <= 20; size++)); do
    for density in 3 5; do
        echo -n "${size},${density},"
        timeout 1h ./build/subiso "Benchmarks/others/${size}_${density}_1.txt" ${size}
        echo
    done
done