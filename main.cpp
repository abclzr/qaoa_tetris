#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstdio>

#include "src/Graph.hpp"
#include "src/GraphMatch.hpp"
#include "src/Mapping.hpp"

using namespace std;
using namespace std::chrono;

// XXX: Move the following pattern graph generation func to a lib
// This cycle here is the "cylce" that excludes swap cycle.
Graph QAOALinearPattern(int n, int cycle=INT_MAX) {
    Graph g(n, false);
    vector<int> pi;
    for (int i = 0; i < n; i++) pi.emplace_back(i);

    int maxCycle = n % 2 == 0 ? 2 * n - 2 : 2 * n - 4;
    int curCycle = 0;
    for (int t = 0; t < maxCycle; t++) {
        if (t % 4 == 0) {
            for (int p = 0; p < n - 1; p += 2) {
                g.add_edge(pi[p], pi[p + 1]);
            }
            curCycle++;
        } else if (t % 4 == 1) {
            for (int p = 1; p < n - 1; p += 2) {
                g.add_edge(pi[p], pi[p + 1]);
            }
            curCycle++;
        } else if (t % 4 == 2) {
            for (int p = 1; p < n - 1; p += 2) {
                swap(pi[p], pi[p + 1]);
            }
        } else if (t % 4 == 3) {
            for (int p = 0; p < n - 1; p += 2) {
                swap(pi[p], pi[p + 1]);
            }
        }
        if (curCycle == cycle) return g;
    }
    if (n % 2 == 1) {
        for (int p = 1; p < n - 1; p += 2) {
            swap(pi[p], pi[p + 1]);
        }
        for (int p = 0; p < n - 1; p += 2) {
            g.add_edge(pi[p], pi[p + 1]);
        }
    }
    return g;
}

int main(int argc, char *argv[]) {
    Graph queryGraph;
    string queryGraphPath = argv[1];
    ifstream queryGraphFile(queryGraphPath);
    if (!queryGraphFile.is_open() || !queryGraph.load_from_file(queryGraphFile)) {
        cerr << "Cannot load query graph file " << queryGraphPath << ".\n";
        return -1;
    }
    queryGraphFile.close();

    int npattern = atoi(argv[2]);

    
    vector<Mapping> result;
    auto start = high_resolution_clock::now();
    int iter = npattern % 2 == 0 ? 1 : 2;
    for (; iter <= npattern; iter++) {
        Graph dataGraph = QAOALinearPattern(npattern, iter); // Pattern graph
        GraphMatch gm(queryGraph, dataGraph);
        result = gm.subgraph_isomorphsim(1);
        if (result.size() > 0) {
            break;
        }
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    printf("%.3f,%d", duration.count() / 1000.0, iter);
    // cout << "Elapsed time: " << duration.count() << " ms." << endl;
    // cout << "subgraph isomorphism is Finished. Found " << result.size() << " results at " << iter << " iteration." << endl;
    // for (int i = 0; i < result.size(); i++) {
    //     // check correctness can be a function in graphmatch
    //     Graph dataGraph = QAOALinearPattern(npattern, iter);
    //     for (auto edge : queryGraph.get_edges()) {
    //         int v1 = result[i].getDataIdx(edge[0]);
    //         int v2 = result[i].getDataIdx(edge[1]);
    //         if (dataGraph.has_edge(v1, v2) == false) {
    //             cerr << "Generated subgraph isomorphism is wrong.";
    //             return -1;
    //         }
    //     }
    //     cout << "subgraph " << i << "(q - d):" << endl;
    //     result[i].print();
    // }


    return 0;
}
