#include "GraphMatch.hpp"  

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>

using namespace boost;
using namespace std;

int GraphMatch::getRootNode() {
    // FIXME use candidate_d(u) / degree_q(u) to get the root.
    // FIXME: add tests.
    // candidate_d(u) is the number of candidates in the data graph.
    // We do not have the label info here thus, c_d(u) is the number of 
    // node v in the data graph that degree_d(v) >= degree_q(u)
    return 0;
}

// Graph::Graph_bst GraphMatch::build_init_CS() {
//     Graph::Graph_bst init_cs;  
//     graph_traits<Graph::Graph_bst>::vertex_iterator ui, ui_end; 
//     for (boost::tie(ui, ui_end) = boost::vertices(queryG_.graph_); ui != ui_end; ++ui) {
//         int u_degree = out_degree(*ui, queryG_.graph_);
//         vector<Graph::Graph_bst::vertex_descriptor> u_candidate_set = queryG_.get_candidate_set(*ui, dataG_);
//         // For each node v in u_candidate_set, we assign an

//     }

//     return init_cs;
// }

// Graph::Graph_bst GraphMatch::build_CS() {
//     Graph::Graph_bst init_cs = build_CS();

//     return init_cs;
// }

vector<vector<pair<int, int>>> GraphMatch::subgraph_isomorphsim() {


    return {};
}