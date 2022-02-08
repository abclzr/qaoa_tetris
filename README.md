
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

## run experiments and upload to gsheet
```bash
cd ../expr
./script.sh [machine_name] > [tmp file]
Example: ./script.sh ilab1 &> result.out
```

The output file should look similar to example.out with no error messages.

The shared google sheet used can be found [here](https://docs.google.com/spreadsheets/d/1WekXMxrAKkBEDEHHF83C5LDKBgRObKVpQj3k6xomSj4/edit?usp=sharing) 

You should follow these steps
1. Add a new sheet and rename it with a prefix "data_", e.g. "data_cpp_base"
2. Copy everything in your output file to the created sheet and split the data into columns.
3. In the menu, there should be a Refresh > Refresh button. Click that after you finish step 2. This will require your permission to run the Google App Script used in this gsheet.
4. Now go to the "Dashboard" sheet, in B1:B5, you should be able to see your new sheet there. Select that and click Refresh again. You can now see the compared results in the table. Highlighted cells are those have the minimum value across all methods.


# QAOA pattern match performance

> **warning**: # of cylces for different methods are not comparable for now, will be updated later!

> python approx, will actively prune search space so the result is not optimal.

> Our version still have only the naive version of get_next_node() and get_extendable_candidates() functions implemented

> Current bottleneck is the get_extendable_candidates()

| size | density | our time (s) | our cycle | python time (s) | python cycle |
|------|---------|----------|------------------|----------|------------------|
| 8    | 2       | 0.006    | 5                | 0.007    | 5                |
| 8    | 3       | 0.005    | 5                | 0.006    | 5                |
| 8    | 4       | 0.003    | 6                | 0.087    | 6                |
| 8    | 5       | 0.009    | 7                | 0.003    | 7                |
| 9    | 2       | 0.002    | 5                | 0.009    | 5                |
| 9    | 3       | 0.029    | 6                | 0.026    | 6                |
| 9    | 4       | 0.012    | 6                | 0.036    | 6                |
| 9    | 5       | 0.007    | 6                | 0.200    | 6                |
| 10   | 2       | 0.009    | 6                | 0.112    | 6                |
| 10   | 3       | 0.035    | 7                | 0.002    | 7                |
| 10   | 4       | 0.015    | 7                | 0.020    | 7                |
| 10   | 5       | 0.061    | 8                | 6.141    | 8                |
| 11   | 2       | 0.045    | 5                | 0.252    | 5                |
| 11   | 3       | 0.054    | 6                | 0.160    | 6                |
| 11   | 4       | 0.052    | 7                | 1.877    | 7                |
| 11   | 5       | 0.172    | 8                | 1.087    | 8                |
| 12   | 2       | 0.035    | 6                | 0.487    | 6                |
| 12   | 3       | 0.462    | 8                | 1.596    | 8                |
| 12   | 4       | 0.222    | 8                | 47.459   | 8                |
| 12   | 5       | 0.267    | 9                | 227.878  | 9                |
| 13   | 2       | 0.713    | 6                | 17.996   | 6                |
| 13   | 3       | 0.278    | 8                | 168.344  | 8                |
| 13   | 4       | 2.348    | 9                | 99.803   | 9                |
| 13   | 5       | 0.511    | 10               | 699.971  | 10               |
| 14   | 2       | 0.042    | 7                | 531.051  | 7                |
| 14   | 3       | 0.41     | 9                | 4.095    | 9                |
| 14   | 4       | 0.05     | 11               |          |                  |
| 14   | 5       | 1.533    | 12               |          |                  |
| 15   | 2       | 2.111    | 9                |          |                  |
| 15   | 3       | 7.145    | 10               |          |                  |
| 15   | 4       | 18.78    | 11               |          |                  |
| 15   | 5       | 25.459   | 12               |          |                  |
| 16   | 2       | 27.74    | 11               |          |                  |
| 16   | 3       | 20.043   | 11               |          |                  |
| 16   | 4       | 110.233  | 12               |          |                  |
| 16   | 5       | 283.987  | 13               |          |                  |
| 17   | 2       | 226.748  | 11               |          |                  |
| 17   | 3       | 137.762  | 12               |          |                  |
| 17   | 4       | 150.364  | 13               |          |                  |
| 17   | 5       | 22.652   | 14               |          |                  |
| 18   | 2       | 75.687   | 12               |          |                  |
| 18   | 3       | 193.078  | 13               |          |                  |
| 18   | 4       | 680.23   | 14               |          |                  |
| 18   | 5       | 1816.44  | 15               |          |                  |
| 19   | 2       | 32.986   | 11               |          |                  |
| 19   | 3       | 581.261  | 13               |          |                  |
| 19   | 4       | 474.372  | 14               |          |                  |
| 19   | 5       | 737.348  | 15               |          |                  |
| 20   | 2       | 268.031  | 12               |          |                  |
| 20   | 3       | 1083.212 | 14               |          |                  |
| 20   | 4       | 3049.715 | 15               |          |                  |
| 20   | 5       |          |                  |          |                  |
