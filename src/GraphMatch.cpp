#include "GraphMatch.hpp"  

#include <unordered_map>

using namespace std;

int GraphMatch::getRootNode() {
    // FIXME use candidate_d(u) / degree_q(u) to get the root.
    // FIXME: add tests.
    // candidate_d(u) is the number of candidates in the data graph.
    // We do not have the label info here thus, c_d(u) is the number of 
    // node v in the data graph that degree_d(v) >= degree_q(u)
    return 0;
}

// XXX: add comments
void GraphMatch::build_init_CS(Graph &initCS, 
                                unordered_map<int, unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv) {

    vector<int> revTopOrder = queryDAG_.get_reversed_topo_order();
    int initCSNodeIndex = 0;
    for (auto u : revTopOrder) {
        set<int> u_candidate_set = queryG_.get_candidate_set(u, dataG_);
        // Each node in u_candidate_set is a node in initCS
        for (auto vi = u_candidate_set.begin(); vi != u_candidate_set.end(); ++vi) {
            initCS.add_node(initCSNodeIndex);
            uv2id[u][*vi] = initCSNodeIndex;
            id2uv[initCSNodeIndex] = make_pair(u, *vi);
            initCSNodeIndex++;
            // Check each u's out edge
            set<int> u_children = queryDAG_.get_neighbors(u);
            for (auto u_primei = u_children.begin(); u_primei != u_children.end(); ++u_primei) {
                set<int> u_child_candidate_set = queryG_.get_candidate_set(*u_primei, dataG_);
                for (auto v_primei = u_child_candidate_set.begin(); v_primei != u_child_candidate_set.end(); ++v_primei) {
                    if (dataG_.has_edge(*vi, *v_primei)) {
                        initCS.add_edge(uv2id[u][*vi], uv2id[*u_primei][*v_primei]);
                    }
                }
            }
        }
    }
}

Graph GraphMatch::build_CS() {
    Graph initCS(true);
    // FIXME:Pre-store all candidate_set to a variable.
    unordered_map<int, unordered_map<int, int>> uv2id;
    unordered_map<int, pair<int, int>> id2uv;
    build_init_CS(initCS, uv2id, id2uv);

    return initCS;
}

vector<vector<pair<int, int>>> GraphMatch::subgraph_isomorphsim() {

    Graph csG = build_CS();

    return {};
}