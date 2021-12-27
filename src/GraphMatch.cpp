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
    // FIXME: chanmge to range based for loop
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

bool GraphMatch::refine_CS_wrapper(Graph &CS, 
                                    unordered_map<int, unordered_map<int, int>> &uv2id,
                                    unordered_map<int, pair<int, int>> &id2uv,
                                    int reversed) {
    bool changed = false;
    if (reversed == 0) { // 0: original dag
        changed =  refine_CS(CS, uv2id, id2uv, queryDAG_);
    } else {
        // XXX: reversed queryDAG won't change, prestore to some variable.
        Graph revCS = CS.generate_reversed_graph();
        Graph revQueryDAG = queryDAG_.generate_reversed_graph();
        changed = refine_CS(revCS, uv2id, id2uv, revQueryDAG);
        CS = revCS.generate_reversed_graph();
    }
    return changed;
}


// FIXME: add comments and rename variables based on paper/comment
bool GraphMatch::refine_CS(Graph &CS, 
                                unordered_map<int, unordered_map<int, int>> &uv2id,
                                unordered_map<int, pair<int, int>> &id2uv,
                                Graph &queryDAG) {
    set<int> filtered_nodes;
    // Iterate all nodes in reversed topo order
    vector<int> iterOrder = queryDAG.get_reversed_topo_order();

    for (auto u : iterOrder) {
        // Get all u's children in q_dag
        set<int> u_children = queryDAG.get_neighbors(u);
        // For each u, we get its C(u) in CS
        auto C_u = uv2id[u];
        for (auto vi : C_u) {
            int v = vi.first, id = vi.second;
            // Get all v's unfiltered children in CS
            set<int> v_children = CS.get_neighbors(v);
            set<int> v_children_u;
            // for each v's child, get its u
            for (auto v_child : v_children) {
                if (filtered_nodes.find(v_child) != filtered_nodes.end()) 
                    continue;
                int v_child_u = id2uv[v_child].first;
                v_children_u.insert(v_child_u);
            }
            if (v_children_u.size() != u_children.size()) {
                filtered_nodes.insert(id);
            }
        }
    }
    if (filtered_nodes.size() == 0) return false;

    // For all the nodes in filtered_nodes, we clean CS, uv2id and id2uv
    CS.remove_nodes(filtered_nodes);
    for (auto id : filtered_nodes) {
        int u = id2uv[id].first, v = id2uv[id].second;
        id2uv.erase(id);
        uv2id[u].erase(v);
    }

    return true;
}


Graph GraphMatch::build_CS() {
    Graph CS(true);
    // FIXME:Pre-store all candidate_set to a variable.
    unordered_map<int, unordered_map<int, int>> uv2id;
    unordered_map<int, pair<int, int>> id2uv;
    build_init_CS(CS, uv2id, id2uv);

    int direction = 1; // 0: reversed
    while (true) {
        if (refine_CS_wrapper(CS, uv2id, id2uv, direction) == false) {
            break;
        }
        direction = 1 - direction;
    }

    return CS;
}

vector<vector<pair<int, int>>> GraphMatch::subgraph_isomorphsim() {

    csG_ = build_CS();

    return {};
}