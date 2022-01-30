#include "GraphMatch.hpp"  
#include "Mapping.hpp"

#include <unordered_map>
#include <algorithm>

using namespace std;

namespace subiso {

int GraphMatch::get_root_node() {
    // FIXME: add tests.
    int min_u = -1;
    float min_w = (float)dataG_.num_nodes();
    for (int u = 0; u < queryG_.num_nodes(); ++u) {
        float w = (float)queryG_.get_candidate_set(u, dataG_).size() / (float)queryG_.degree(u);
        if (w <= min_w) {
            min_u = u;
            min_w = w;
        }
    }
    return min_u;
}


pair<int, unordered_set<int>> GraphMatch::get_next_node(Mapping &M, 
                                Graph &queryDAG,
                                Graph &revQueryDAG,
                                set<int> &expendable_u,
                                Graph &CS,
                                unordered_map<int, unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv,
                                unordered_map<int,int> &weightArray) {
    //looking for expendable vertices
    vector<int> iterOrder = queryDAG.get_topo_order();

    //looking for candidates for each expendable node
    unordered_map<int, unordered_set<int>> u_candidates_map;
    for (auto u : expendable_u) {
        unordered_set<int> expendable_id;

        //get candidates of u
        for(auto v_id : uv2id[u]) {
            //make sure all candidates are not mapped
            if(M.findDataIdx(v_id.first)) continue;
            
            expendable_id.insert(v_id.second);
        }
        
        auto u_parents = revQueryDAG.get_neighbors(u);
        for(auto p : u_parents) {
            int v_p = M.getDataIdx(p); 
            int id_v_p = uv2id[p][v_p];
            auto id_neigh_v_p = CS.get_neighbors(id_v_p);
            for (auto it = expendable_id.begin(); it != expendable_id.end();) {
                if (id_neigh_v_p.find(*it) == id_neigh_v_p.end()) {
                    it = expendable_id.erase(it);
                } else {
                    ++it;
                }
            }
        }

        u_candidates_map[u] = expendable_id;
    }

    //calculating weight for each u in expendable_u
    //using weight to find a u with min weight from expendable_u. 
    int u_weight = INT_MAX;
    int u_min = -1;
    for(auto u : expendable_u) {
        int weight = 0;
        for(auto id : u_candidates_map[u]){
            weight += weightArray[id];
        }
        if (u_weight > weight){
            u_weight = weight;
            u_min = u;
        }
    }

    //convert id of uv back to v
    unordered_set<int> u_candidates;
    for(auto id : u_candidates_map[u_min]){
        u_candidates.insert(id2uv[id].second);
    }

    if(u_min < 0)
        return make_pair(-1,u_candidates);

    return make_pair(u_min,u_candidates);
}


// XXX: add comments
void GraphMatch::build_init_CS(Graph &initCS, 
                                unordered_map<int, unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv) {

    vector<int> revTopOrder = queryDAG_.get_reversed_topo_order();
    int root = get_root_node();
    // root = -1;
    int initCSNodeIndex = 0;
    for (auto u : revTopOrder) {
        unordered_set<int> u_candidate_set = queryG_.get_candidate_set(u, dataG_, root);
        // Each node in u_candidate_set is a node in initCS
        for (auto v : u_candidate_set) {
            initCS.add_node(initCSNodeIndex);
            uv2id[u][v] = initCSNodeIndex;
            id2uv[initCSNodeIndex] = make_pair(u, v);
            initCSNodeIndex++;
            // Check each u's out edge
            unordered_set<int> u_children = queryDAG_.get_neighbors(u);
            for (auto u_prime : u_children) {
                unordered_set<int> u_child_candidate_set = queryG_.get_candidate_set(u_prime, dataG_, root);
                for (auto v_prime : u_child_candidate_set) {
                    if (dataG_.has_edge(v, v_prime)) {
                        initCS.add_edge(uv2id[u][v], uv2id[u_prime][v_prime]);
                    }
                }
            }
        }
    }
}

bool GraphMatch::refine_CS_wrapper(Graph &CS, 
                                    unordered_map<int, unordered_map<int, int>> &uv2id,
                                    unordered_map<int, pair<int, int>> &id2uv,
                                    Graph &queryDAG,
                                    int reversed) {
    bool changed = false;
    if (reversed == 0) { // 0: original dag
        changed =  refine_CS(CS, uv2id, id2uv, queryDAG);
    } else {
        // XXX: reversed queryDAG won't change, prestore to some variable.
        Graph revCS = CS.generate_reversed_graph();
        Graph revQueryDAG = queryDAG.generate_reversed_graph();
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
    unordered_set<int> filtered_nodes;
    // Iterate all nodes in reversed topo order
    vector<int> iterOrder = queryDAG.get_reversed_topo_order();

    for (auto u : iterOrder) {
        // Get all u's children in q_dag
        unordered_set<int> u_children = queryDAG.get_neighbors(u);
        // For each u, we get its C(u) in CS
        auto C_u = uv2id[u];
        for (auto vi : C_u) {
            int v = vi.first, id = vi.second;
            // Get all v's unfiltered children in CS
            unordered_set<int> v_children = CS.get_neighbors(id);
            unordered_set<int> v_children_u;
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


void GraphMatch::build_weight_array(unordered_map<int, int>  &weightArray,
                                    Graph &queryDAG, 
                                    Graph &CS,
                                    unordered_map<int, unordered_map<int, int>> &uv2id,
                                    unordered_map<int, pair<int, int>> &id2uv) {
    // Iterate all nodes in queryDAG in reversed topo order.
    // Case 1: If a node u has a child u' that has in_degree > 1. All v in C(u) has w(u, v) = 1
    // Case 2: Else for each v in C(u), sum w(u', v') for each u' and v' in C(u') while v' is v's child.
    // w(u, v) = the minimum sum w(u', v') of all u'.
    vector<int> iterOrder = queryDAG.get_reversed_topo_order();
    for (int u : iterOrder) {
        unordered_set<int> u_children = queryDAG.get_neighbors(u);
        auto C_u = uv2id[u];
        if (all_of(u_children.begin(), u_children.end(), 
            [&](int u_prime){
                return queryDAG.in_degree(u_prime) > 1;
                })) { // Case 1
            // Set w(u, v) to 1 for all v in C(u).
            for (auto vi : C_u) {
                int v = vi.first, id = vi.second;
                weightArray[id] = 1;
            }
        } else {
            for (auto vi : C_u) {
                int v = vi.first, id = vi.second;
                unordered_map<int, int> weights;

                unordered_set<int> id_children = CS.get_neighbors(id);
                for (auto id_prime : id_children) {
                    int u_prime = id2uv[id_prime].first;
                    weights[u_prime] += weightArray[id_prime];
                }
                weightArray[id] = (*min_element(weights.begin(), weights.end(), 
                   [](const auto& lhs, const auto& rhs){ return lhs.second < rhs.second;})).second;
            }
        }
    }
}


void GraphMatch::build_CS() {
    csG_ = Graph(true);
    // FIXME:Pre-store all candidate_set to a variable.
    build_init_CS(csG_, uv2id_, id2uv_);

    int direction = 1; // 0: reversed
    int iteration = 1;
    while (true) {
        // csG_.print_adjList();
        // cout << "refine iteration " << iteration << endl;
        // cout << "nodes before: " << csG_.num_nonempty_nodes() << endl;
        if (refine_CS_wrapper(csG_, uv2id_, id2uv_, queryDAG_, direction) == false) {
            break;
        }
        // cout << "nodes after: " << csG_.num_nonempty_nodes() << endl;
        direction = 1 - direction;
        iteration += 1;
    }

    build_weight_array(weightArray_, queryDAG_, csG_, uv2id_, id2uv_);
}


bool GraphMatch::backtrack(Mapping &M,
                            vector<Mapping> &allM_prime, 
                            set<int> expendable_u,
                            unordered_map<int,int> indegrees,
                            int count) {
    if (M.size() == queryG_.num_nodes()) {
        allM_prime.emplace_back(M);
        return allM_prime.size() < count;
    } else if (M.size() == 0) {
        int u = get_root_node();
        for (auto nbr : queryDAG_.get_neighbors(u)) {
            indegrees[nbr] += 1;
            if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                expendable_u.insert(nbr);
            }
        }
        auto C_u = uv2id_[u];
        for (auto vi : C_u) {
            int v = vi.first;
            M.update(u, v);
            if (backtrack(M, allM_prime, expendable_u, indegrees, count) == false) return false;
            M.eraseByQueryIdx(u);
        }
        for (auto nbr : queryDAG_.get_neighbors(u)) {
            if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                expendable_u.erase(nbr);
            }
            indegrees[nbr] -= 1;
        }
    } else {
        pair<int, unordered_set<int>> u_candidates = get_next_node(M, 
                                queryDAG_,
                                revQueryDAG_,
                                expendable_u,
                                csG_,
                                uv2id_, 
                                id2uv_,
                                weightArray_); 

        int u = u_candidates.first;
        expendable_u.erase(u);

        // update in_degree & expendable_u
        for (auto nbr : queryDAG_.get_neighbors(u)) {
            indegrees[nbr] += 1;
            if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                expendable_u.insert(nbr);
            }
        }
        
        for (auto v : u_candidates.second) {
            if (M.findDataIdx(v) == false) {
                M.update(u, v);
                if (backtrack(M, allM_prime, expendable_u, indegrees, count) == false) return false;
                M.eraseByQueryIdx(u);
            }
        }

        for (auto nbr : queryDAG_.get_neighbors(u)) {
            if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                expendable_u.erase(nbr);
            }
            indegrees[nbr] -= 1;
        }
    }
    return true;
}


vector<Mapping> GraphMatch::subgraph_isomorphsim(int count) {
    if (dataG_.num_edges() < queryG_.num_edges()) return {};

    build_CS();

    Mapping M(queryG_.num_nodes());
    vector<Mapping> allM_prime;
    set<int> expendable_u;
    unordered_map<int, int> indegrees;
    backtrack(M, allM_prime, expendable_u, indegrees, count);

    if (allM_prime.size() >= count) {
        return vector<Mapping>(allM_prime.begin(), allM_prime.begin() + count);
    } else {
        return allM_prime;
    }

}

} // namespace: subiso