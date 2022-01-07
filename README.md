
# subgraph_isomorphism
## compile
```zsh
mkdir build
cd build
cmake ..
make
```
## run
```
./build/subiso [query_graph_path] [qaoa_pattern_size]
./build/subiso Benchmarks/others/8_3_1.txt 8
```

# performance profiling

Only tested via local vscode with seastar as the remote server.
Use gperf with google pprof.

```zsh
cd build
CPUPROFILE=prof.out ./subiso_profile 
~/go/bin/pprof -http=":" ./subiso prof.out
```

# QAOA pattern match performance

> **warning**: # of cylces for different methods are not comparable for now, will be updated later!

> python approx, will actively prune search space so the result is not optimal.

> Our version still have only the naive version of get_next_node() and get_extendable_candidates() functions implemented

> Current bottleneck is the get_extendable_candidates()

| size | density | our time (s) | our cycle | python approx time (s) | python approx cycle | python time (s) | python cycle |
|------|---------|--------------|-----------|------------------------|---------------------|-----------------|--------------|
| 8    | 3       | 0.003        | 8         | 0.134                  | 10                  | 0.029           | 8            |
| 9    | 3       | 0.008        | 8         | 0.137                  | 9                   | 0.083           | 9            |
| 10   | 3       | 0.022        | 12        | 0.163                  | 13                  | 0.558           | 12           |
| 11   | 3       | 0.734        | 12        | 0.283                  | 12                  | 0.966           | 11           |
| 12   | 3       | 0.760        | 14        | 0.267                  | 15                  | 0.944           | 13           |
| 13   | 3       | 0.802        | 16        | 0.279                  | 18                  | 3.220           | 16           |
| 14   | 3       | 2.504        | 16        | 0.390                  | 20                  | 5.211           | 17           |
| 15   | 3       | 36.924       | 18        | 0.723                  | 21                  | 21.926          | 17           |
| 8    | 5       | 0.007        | 10        | 0.097                  | 10                  | 0.017           | 10           |
| 9    | 5       | 0.005        | 10        | 0.141                  | 10                  | 0.024           | 10           |
| 10   | 5       | 0.011        | 14        | 0.181                  | 10                  | 0.197           | 14           |
| 11   | 5       | 0.172        | 14        | 0.236                  | 17                  | 0.716           | 13           |
| 12   | 5       | 0.449        | 18        | 0.458                  | 16                  | 0.245           | 17           |
| 13   | 5       | 3.299        | 18        | 0.548                  | 20                  | 2.473           | 18           |
| 14   | 5       | 9.266        | 20        | 0.608                  | 21                  | 8.761           | 20           |
| 15   | 5       | 10.872       | 22        | 0.763                  | 22                  | 51.636          | 22           |