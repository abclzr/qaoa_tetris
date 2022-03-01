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

    if (argc != 4) {
        cerr << "./subiso [Query Graph] [QAOA Pattern Size] [direction]\n";
        cerr << "E.g. ./subiso ../Benchmarks/16_2_0.txt 16 0\n";
        return -1;
    }

    Graph queryGraph;
    string queryGraphPath = argv[1];
    ifstream queryGraphFile(queryGraphPath);
    if (!queryGraphFile.is_open() || !queryGraph.load_from_file(queryGraphFile)) {
        cerr << "Cannot load query graph file " << queryGraphPath << ".\n";
        return -1;
    }
    queryGraphFile.close();

    int npattern = atoi(argv[2]);
    int direction = atoi(argv[3]);

    vector<BiMap> result;
    int iter, iter_start, iter_end, iter_delta;
    if (direction == 0) {
        iter_start = npattern % 2 == 0 ? 1 : 2;
        iter_end = npattern + 1;
        iter_delta = 1;
    } else {
        iter_start = npattern;
        iter_end = 0;
        iter_delta = -1;        
    }

    std::chrono::_V2::system_clock::time_point total_start, total_stop;
    total_start = high_resolution_clock::now();
#ifndef NDEBUG
    std::chrono::_V2::system_clock::time_point start, stop;
#endif

    for (iter = iter_start; iter != iter_end; iter += iter_delta) {
#ifndef NDEBUG
        printf("iter: %d; ", iter);
        start = high_resolution_clock::now();
#endif
        Graph dataGraph = QAOALinearPattern(npattern, iter); // Pattern graph
#ifndef NDEBUG
        stop = high_resolution_clock::now();
        auto pattern_prepare_time = duration_cast<milliseconds>(stop - start).count();
        printf("pattern graph prepare: %ld ms; ", pattern_prepare_time);

        start = high_resolution_clock::now();
#endif
        GraphMatch gm(queryGraph, dataGraph);
#ifndef NDEBUG
        stop = high_resolution_clock::now();
        auto gm_init_time = duration_cast<milliseconds>(stop - start).count();
        printf("init GraphMatch class: %ld ms; ", gm_init_time);

        start = high_resolution_clock::now();
#endif
        result = gm.subgraph_isomorphsim(1);
#ifndef NDEBUG
        stop = high_resolution_clock::now();
        auto subiso_time = duration_cast<milliseconds>(stop - start).count();

        // printf("# results: %ld;", result.size());
        printf("explore elapsed: %ld ms; bt count: %d\n", subiso_time, gm.bt_count);
#endif
        if (direction == 0 && result.size() > 0) {
            break;
        } else if (direction == 1 && result.size() == 0) {
            iter += 1;
            break;
        }
    }

    total_stop = high_resolution_clock::now();
    auto total_duration = duration_cast<milliseconds>(total_stop - total_start).count();
    printf("%.3fs,%d", total_duration / 1000.0, iter);


    return 0;
}
