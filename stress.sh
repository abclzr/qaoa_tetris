#!/usr/bin/env bash

for size in 8 9 10 11 12 13 14 15 20 30 40 50; do
    for density in 3 5 8; do
        echo -n "${size},${density},"
        timeout 10s ./build/subiso "Benchmarks/others/${size}_${density}_1.txt" ${size}
        echo
    done
done