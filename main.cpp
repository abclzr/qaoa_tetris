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

    if (argc != 5 && argc != 4) {
        cerr << "./subiso [Query Graph] [QAOA Pattern Size] [direction] [maxK = 3]\n";
        cerr << "E.g. ./subiso ../Benchmarks/others/16_2_0.txt 16 0 3\n";
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
    int maxK = argc == 4 ? 3 : atoi(argv[4]);
    maxK = maxK < 3 ? 3 : maxK;

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
    
    // for (int i = 0; i < npattern; i++) {
    int total_bt_count = 0;
    std::chrono::_V2::system_clock::time_point total_start, total_stop;
    total_start = high_resolution_clock::now();
#ifndef NDEBUG
    std::chrono::_V2::system_clock::time_point start, stop;

    start = high_resolution_clock::now();
#endif
    
    Graph dataGraph = QAOALinearPattern(npattern, iter_start); // Pattern graph
    dataGraph.generate_edge_checker();
    GraphMatch gm(queryGraph, dataGraph, maxK);
    
#ifndef NDEBUG
    stop = high_resolution_clock::now();
    auto gm_init_time = duration_cast<milliseconds>(stop - start).count();
    printf("init GraphMatch class: %ld ms\n", gm_init_time);
#endif
    
    for (iter = iter_start + 1; iter != iter_end; iter += iter_delta) {
#ifndef NDEBUG
        printf("iter: %d; ", iter);
        fflush(stdout);
        start = high_resolution_clock::now();
#endif
        dataGraph = QAOALinearPattern(npattern, iter); // Pattern graph
        dataGraph.generate_edge_checker();
#ifndef NDEBUG
        stop = high_resolution_clock::now();
        auto pattern_prepare_time = duration_cast<milliseconds>(stop - start).count();
        printf("pattern graph prepare: %ld ms;\n", pattern_prepare_time);
        fflush(stdout);
#endif

        int root = gm.get_root_node(queryGraph);
        auto rootCandidates = queryGraph.get_candidate_set(root, dataGraph);

        rootCandidates = {-1};

        for (auto rootCandidate : rootCandidates) {
#ifndef NDEBUG
            start = high_resolution_clock::now();
#endif
            // gm.update_data_G(dataGraph);
            gm = GraphMatch(queryGraph, dataGraph, maxK, root, rootCandidate);
            if (gm.has_subgraph() == false) break;
#ifndef NDEBUG
            stop = high_resolution_clock::now();
            auto gm_update_time = duration_cast<milliseconds>(stop - start).count();
            printf("update GraphMatch class: %ld ms; ", gm_update_time);
            fflush(stdout);
#endif
            if (gm.has_subgraph() == false) break;
#ifndef NDEBUG
            start = high_resolution_clock::now();
#endif
            result = gm.subgraph_isomorphsim(1);
#ifndef NDEBUG
            stop = high_resolution_clock::now();
            auto subiso_time = duration_cast<milliseconds>(stop - start).count();

            // printf("# results: %ld;", result.size());
            printf("\troot %d; root candidate: %d; ", root, rootCandidate);
            printf("\texplore elapsed: %ld ms; bt count: %d; root index: %d\n", subiso_time, gm.bt_count, gm.get_root_index());
#endif
            total_bt_count += gm.bt_count;
            if (direction == 0 && result.size() > 0) {
                // result[0].print();
                break;
            } else if (direction == 1 && result.size() == 0) {
                iter += 1;
                break;
            }
        }

        if (direction == 0 && result.size() > 0) {
            // result[0].print();
            break;
        } else if (direction == 1 && result.size() == 0) {
            iter += 1;
            break;
        }

    }

    total_stop = high_resolution_clock::now();
    auto total_duration = duration_cast<milliseconds>(total_stop - total_start).count();
    // printf("%.3f,%d", total_duration / 1000.0, iter);
    printf("%.3f,%d,%d", total_duration / 1000.0, iter,total_bt_count);
    // printf("%d:%.3fs,%d\n", i, total_duration / 1000.0, iter);
    // }
#ifndef NDEBUG
    printf("\nChecking %ld results...", result.size());
    for (int i = 0; i < result.size(); i++) {
        auto M = result[i];
        // For each edge u u' in the queryG, we can find edge M[u], M[u'] in dataG
        for (auto edge : queryGraph.get_edges()) {
            int u = edge[0], u_prime = edge[1];
            if (dataGraph.has_edge(M.getValueByKey(u), M.getValueByKey(u_prime)) == false) {
                printf("\nQuery graph has edge (%d, %d), ", u, u_prime);
                printf("with mapping q:%d->d:%d and q:%d->d:%d, ", u, M.getValueByKey(u), u_prime, M.getValueByKey(u_prime));
                printf("Data graph does not have edge (%d, %d).\n", M.getValueByKey(u), M.getValueByKey(u_prime));
                printf("Query Graph:\n");
                queryGraph.print_adjList();
                printf("Data Graph:\n");
                dataGraph.print_adjList();
                return -1;
            }
        }
    }
    printf("Done.\n");
#endif
    return 0;
}
