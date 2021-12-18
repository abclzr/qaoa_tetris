#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>

#include "src/Graph.hpp"

using namespace std;

int main(int, char**) {
    ifstream graph_file("../test/graph/line2.txt");
    if (!graph_file.is_open()) {
        cerr << "graph file line2.txt does not exist.\n";
        return(EXIT_FAILURE);
    } else {
        Graph g;
        g.load_from_file(graph_file);

        graph_file.close();
    }

    return 0;
}
