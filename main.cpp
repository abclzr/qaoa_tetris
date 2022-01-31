#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstdio>

#include "Graph.hpp"
#include "GraphMatch.hpp"
#include "BiMap.hpp"

using namespace std;
using namespace std::chrono;
using namespace subiso;
using namespace qaoagraph;
using namespace bimap;

// XXX: Move the following pattern graph generation func to a lib
// This cycle here is the "cycle" that excludes swap cycle.
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

    
    vector<BiMap> result;
    int iter = npattern % 2 == 0 ? 1 : 2;
    int total_duration = 0;
    for (; iter <= npattern; iter++) {
        Graph dataGraph = QAOALinearPattern(npattern, iter); // Pattern graph
        GraphMatch gm(queryGraph, dataGraph);
        auto start = high_resolution_clock::now();
        result = gm.subgraph_isomorphsim(1);
        auto stop = high_resolution_clock::now();
        total_duration += duration_cast<milliseconds>(stop - start).count();
        // cout << "iter " << iter << " Elapsed time: " << duration << " ms." << endl;
        if (result.size() > 0) {
            break;
        }
    }

    printf("%.3f,%d", total_duration / 1000.0, iter);

    return 0;
}
