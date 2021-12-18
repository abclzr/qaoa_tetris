#include "Graph.hpp"

#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

using namespace boost;
	
Graph::Graph()
{

}
	
Graph::~Graph()
{
	
}

bool Graph::load_from_file(ifstream &graph_file) {
    int numNodes, numEdges;
    graph_file >> numNodes >> numEdges;
    graph_ = Graph_bst(numNodes);
    
    // TODO: check if the edge number is correct.
    for (int i = 0; i < numEdges; i++) {
        int n1, n2;
        graph_file >> n1 >> n2;
        if (n1 >= numNodes || n2 >= numNodes) {
            return false;
        }
        add_edge(n1, n2, graph_);
    }

    return true;
}