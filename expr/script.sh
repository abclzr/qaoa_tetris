#!/usr/bin/env bash

machine=$1

echo "${machine}"
echo "graph_size, graph_density, graph_index, graph_label, time (s), qaoa_cycles"

for size in 8 9 10 11 12 13 14 15 16 17 18 19 20; do
# for size in 21 22 23; do
    for density in 3 5; do
        for graph_index in {1..1}; do
                graph_label="${size}_${density}_${graph_index}"
            echo -n "${size}, ${density}, ${graph_index}, ${graph_label}, "
            ./../build/subiso "../Benchmarks/others/${size}_${density}_${graph_index}.txt" ${size} 0
            echo
        done
    done
done