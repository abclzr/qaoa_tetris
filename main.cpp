#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <vector>

#include "src/Graph.hpp"
#include "src/GraphMatch.hpp"
#include "src/Mapping.hpp"

using namespace std;

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
        } else if (t % 4 == 1) {
            for (int p = 1; p < n - 1; p += 2) {
                g.add_edge(pi[p], pi[p + 1]);
            }
        } else if (t % 4 == 2) {
            for (int p = 1; p < n - 1; p += 2) {
                swap(pi[p], pi[p + 1]);
            }
        } else if (t % 4 == 3) {
            for (int p = 0; p < n - 1; p += 2) {
                swap(pi[p], pi[p + 1]);
            }
        }
        curCycle++;
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

int main(int, char**) {
    Graph queryGraph;
    string queryGraphPath = "../test/graph/10_1_0.txt";
    ifstream queryGraphFile(queryGraphPath);
    if (!queryGraphFile.is_open() || !queryGraph.load_from_file(queryGraphFile)) {
        cerr << "Cannot load query graph file ../test/graph/10_1_0.txt.\n";
        return -1;
    }
    queryGraphFile.close();

    Graph dataGraph = QAOALinearPattern(10); // Pattern graph

    GraphMatch gm(queryGraph, dataGraph);
    vector<Mapping> result;
    result = gm.subgraph_isomorphsim(1);
    cout << "subgraph isomorphsim finds " << result.size() << " results" << endl;
    for (int i = 0; i < result.size(); i++) {
        cout << "subgraph " << i << "(d - q):" << endl;
        result[i].print();
    }


    return 0;
}
