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

    vector<int> degrees = {3, 5};
    vector<int> graph_sizes = {8, 9, 10, 11, 12 ,13, 14, 15};

    unordered_map<int, unordered_map<int, pair<int, int>>> results;

    for (int g_size : graph_sizes) {
        int npattern = g_size;
        for (int degree : degrees) {
            // Query Graph
            Graph queryGraph;
            string queryGraphPath = "../Benchmarks/others/" + to_string(g_size) + "_" + to_string(degree) + "_1.txt";
            ifstream queryGraphFile(queryGraphPath);
            if (!queryGraphFile.is_open() || !queryGraph.load_from_file(queryGraphFile)) {
                cerr << "Cannot load query graph file " << queryGraphPath << ".\n";
                return -1;
            }
            queryGraphFile.close();
            cout << "graph size: " << g_size << " degree: " << degree << " ";
            cout << to_string(g_size) + "_" + to_string(degree) + "_1.txt" << endl;

            vector<BiMap> result;
            
            int iter = npattern % 2 == 0 ? 1 : 2;
            int duration = 0;
            for (; iter <= npattern; iter++) {
                Graph dataGraph = QAOALinearPattern(npattern, iter); // Pattern graph
                GraphMatch gm(queryGraph, dataGraph);
                auto start = high_resolution_clock::now();
                result = gm.subgraph_isomorphsim(1);
                auto stop = high_resolution_clock::now();
                duration += duration_cast<milliseconds>(stop - start).count();
                if (result.size() > 0) {
                    break;
                }
            }
            
            cout << "Elapsed time: " << duration << " ms." << endl;
            cout << "subgraph isomorphism is Finished. Found " << result.size() << " results at " << iter << " iteration." << endl;

            for (int i = 0; i < result.size(); i++) {
                // check correctness can be a function in graphmatch
                Graph dataGraph = QAOALinearPattern(npattern, iter);
                for (auto edge : queryGraph.get_edges()) {
                    int v1 = result[i].getValueByKey(edge[0]);
                    int v2 = result[i].getValueByKey(edge[1]);
                    if (dataGraph.has_edge(v1, v2) == false) {
                        cerr << "Generated subgraph isomorphism is wrong.";
                        return -1;
                    }
                }
                cout << "subgraph " << i << "(q - d):" << endl;
                result[i].print();
            }

            results[g_size][degree] = make_pair(duration, iter);
        }
    }

    // Print in Markdown table format.
    printf("| size | density | our time (s) | our iter |\n");
    printf("| --- | --- | --- | --- |\n");
    for (int degree : degrees) {
        for (int g_size : graph_sizes) {
            printf("| %d | %d | %.3f | %d |\n", g_size, degree, results[g_size][degree].first / 1000.0, results[g_size][degree].second);
        }
    }

    return 0;
}
