#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include <vector>

#include "src/Graph.hpp"
#include "src/GraphMatch.hpp"

using namespace std;

int main(int, char**) {
    ifstream qgraph_file("../test/graph/triangle.txt");
    ifstream dgraph_file("../test/graph/10_1_0.txt");
    if (!qgraph_file.is_open()) {
        cerr << "query graph file triangle.txt does not exist.\n";
        return(EXIT_FAILURE);
    }
    if (!dgraph_file.is_open()) {
        cerr << "data graph file 10_1_0.txt does not exist.\n";
        return(EXIT_FAILURE);        
    }

    Graph qgraph, dgraph;
    if (!qgraph.load_from_file(qgraph_file)) {
        cerr << "Cannot load graph file triangle.txt.\n";
    }
    if (!dgraph.load_from_file(dgraph_file)) {
        cerr << "Cannot load graph file 10_1_0.txt.\n";
    }

    GraphMatch gm(qgraph, dgraph);
    vector<vector<pair<int, int>>> result;
    result = gm.subgraph_isomorphsim();
    cout << "subgraph isomorphsim finds " << result.size() << " results" << endl;
    for (int i = 0; i < result.size(); i++) {
        cout << "subgraph " << i << "(q - d):" << endl;
        for (auto match : result[i]) {
            cout << "(" << match.first << " - " << match.second << ") | ";
        }
        cout << endl;
    }

    qgraph_file.close();
    dgraph_file.close();

    return 0;
}
