
# subgraph_isomorphism

```zsh
mkdir build
cd build
cmake ..
make
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

| size | degree | time (s) | cycle |
| --- | --- | --- | --- |
| 8 | 3 | 0.006 | 5 |
| 9 | 3 | 0.009 | 5 |
| 10 | 3 | 0.025 | 7 |
| 11 | 3 | 0.776 | 7 |
| 12 | 3 | 0.791 | 8 |
| 13 | 3 | 0.842 | 9 |
| 14 | 3 | 2.583 | 9 |
| 15 | 3 | 37.775 | 10 |
| 8 | 5 | 0.010 | 6 |
| 9 | 5 | 0.006 | 6 |
| 10 | 5 | 0.014 | 8 |
| 11 | 5 | 0.193 | 8 |
| 12 | 5 | 0.469 | 10 |
| 13 | 5 | 3.375 | 10 |
| 14 | 5 | 9.551 | 11 |
| 15 | 5 | 10.938 | 12 |
