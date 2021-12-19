#include "Graph.hpp"

#include <iostream>
#include <boost/graph/copy.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>


using namespace boost;
using namespace std;
	
Graph::Graph(Graph_bst graph) {
    copy_graph(graph, graph_);
}
	
Graph::~Graph() {
	
}

bool Graph::load_from_file(ifstream &graphFile) {
    int numNodes, numEdges;
    graphFile >> numNodes >> numEdges;
    graph_ = Graph_bst(numNodes);
    
    // FIXME: check if the edge number is correct.
    for (int i = 0; i < numEdges; i++) {
        int n1, n2;
        graphFile >> n1 >> n2;
        if (n1 >= numNodes || n2 >= numNodes) {
            return false;
        }
        add_edge(n1, n2, graph_);
    }

    return true;
}

Graph::DGraph_bst Graph::generate_dag(int rootIndex) {
    // Traverse the graph using bfs starts from the root, for every neighbor node nn of the 
    // current node n, we add directed edge (n, nn)
    // TODO: The original DAF algorithms prioritize nodes in the same level that have larger label frequency.
    // We use default(explored) order for now.

    MyVisitor vis(num_vertices(graph_), dgraph_);
    breadth_first_search(graph_, vertex(rootIndex, graph_), visitor(vis));

    // Print graph
    // TODO: organize the following into a print graph func.
    // graph_traits <DGraph_bst>::vertex_iterator i, end;
    // graph_traits <DGraph_bst>::adjacency_iterator ai, a_end;

    // for (boost::tie(i, end) = vertices(dgraph_); i != end; ++i) {
    //     std::cout << *i << ": ";

    //     for (boost::tie(ai, a_end) = adjacent_vertices(*i, dgraph_); ai != a_end; ++ai) {
    //         std::cout << *ai;
    //         if (boost::next(ai) != a_end)
    //             std::cout << ", ";
    //     }
    //     std::cout << std::endl;
    // }

    return dgraph_;
}