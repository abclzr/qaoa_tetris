#include "GraphMatch.hpp"  

#include <unordered_map>
#include <algorithm>

using namespace std;

int GraphMatch::get_root_node() {
    // FIXME use candidate_d(u) / degree_q(u) to get the root.
    // FIXME: add tests.
    // candidate_d(u) is the number of candidates in the data graph.
    // We do not have the label info here thus, c_d(u) is the number of 
    // node v in the data graph that degree_d(v) >= degree_q(u)
    return 0;
}


int GraphMatch::get_next_node(unordered_map<int, int> &M, 
                                Graph &queryDAG,
                                Graph &CS,
                                unordered_map<int, unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv) {
    // FIXME: Use Sec 5.2 adaptive matching order (path size order) to find the next node.
    // FIXME: Currently just return a random node from all available (dependency resolved) nodes.
    vector<int> iterOrder = queryDAG.get_topo_order();
    set<int> mapped_u;
    for (auto it : M) mapped_u.insert(it.second);
    for (auto u : iterOrder) {
        if (mapped_u.find(u) == mapped_u.end()) {
            return u;
        }
    }
    return -1; // Should not reach here.
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
            set<int> v_children = CS.get_neighbors(id);
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
        set<int> u_children = queryDAG.get_neighbors(u);
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

                set<int> id_children = CS.get_neighbors(id);
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
    while (true) {
        // csG_.print_adjList();
        if (refine_CS_wrapper(csG_, uv2id_, id2uv_, queryDAG_, direction) == false) {
            break;
        }
        direction = 1 - direction;
    }

    build_weight_array(weightArray_, queryDAG_, csG_, uv2id_, id2uv_);
}


// FIXME: double check if M and M_prime both needed
void GraphMatch::backtrack(unordered_map<int, int> &M, 
                            unordered_map<int, int> &M_prime, 
                            vector<unordered_map<int, int>> &allM_prime, 
                            int count) {
    if (M_prime.size() == queryG_.num_nodes()) {
        if (allM_prime.size() < count) {
            allM_prime.emplace_back(M_prime);
        }
        return;
    } else if (M.size() == 0) {
        int u = get_root_node();
        auto C_u = uv2id_[u];
        for (auto vi : C_u) {
            int v = vi.first;
            M[v] = u;
            M_prime[u] = v;
            backtrack(M, M_prime, allM_prime, count);
            M.erase(v);
            M_prime.erase(u);
        }
    } else {
        int u = get_next_node(M, queryDAG_, csG_, uv2id_, id2uv_);
        // Get extendable candidates
        // XXX: make the following a seperate function and add tests
        // XXX: improve the implementation by using set_intersection
        auto u_parents = revQueryDAG_.get_neighbors(u);
        vector<set<int>> EC_u_parents(u_parents.size());
        int i = 0;
        for (auto u_parent : u_parents) {
            int u_parent_v = M_prime[u_parent];
            int u_parent_id = uv2id_[u_parent][u_parent_v];
            auto u_parent_id_nbr_id = csG_.get_neighbors(u_parent_id);
            for (auto id : u_parent_id_nbr_id) {
                EC_u_parents[i].insert(id2uv_[id].second);
            }
            i += 1;
        }
        unordered_map<int, int> counter;
        for (auto EC_u_parent : EC_u_parents) {
            for (auto v : EC_u_parent) {
                counter[v]++;
            }
        }
        set<int> EC_u;
        for (auto it : counter) {
            int v = it.first, count = it.second;
            if (count == u_parents.size()) {
                EC_u.insert(v);
            }
        }

        for (auto v : EC_u) {
            if (M.find(v) == M.end()) {
                M[v] = u;
                M_prime[u] = v;
                backtrack(M, M_prime, allM_prime, count);
                M.erase(v);
                M_prime.erase(u);
            }
        }        
    }
}


vector<unordered_map<int, int>> GraphMatch::subgraph_isomorphsim(int count) {

    build_CS();

    unordered_map<int, int> M; // v to u
    unordered_map<int, int> M_prime; // u to v
    vector<unordered_map<int, int>> allM_prime;
    backtrack(M, M_prime, allM_prime, count);

    if (allM_prime.size() >= count) {
        return vector<unordered_map<int, int>>(allM_prime.begin(), allM_prime.begin() + count);
    } else {
        return allM_prime;
    }

}