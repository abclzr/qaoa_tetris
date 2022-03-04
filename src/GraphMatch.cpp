#include "GraphMatch.hpp"  
#include "BiMap.hpp"

#include <unordered_map>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace qaoagraph;
using namespace bimap;

namespace subiso {

int GraphMatch::get_root_node() {
    // FIXME: add tests.
    // int min_u = -1;
    // float min_w = (float)dataG_.num_nodes();
    // for (int u = 0; u < queryG_.num_nodes(); ++u) {
    //     float w = (float)queryG_.get_candidate_set(u, dataG_).size() / (float)queryG_.degree(u);
    //     if (w <= min_w) {
    //         min_u = u;
    //         min_w = w;
    //     }
    // }
    // return min_u;

    int root_node = -1;
    vector<int> degrees(queryG_.num_nodes());
    for (int i = 0; i < queryG_.num_nodes(); ++i) {
        degrees[i] = queryG_.degree(i);
    }
    float max_u = -1;
    for (int i = 0; i < queryG_.num_nodes(); ++i) {
        float w = (float)degrees[i];
        for (auto nbr : queryG_.get_neighbors(i)) {
            w += (float)degrees[nbr];
        }
        if (w / (1 + queryG_.get_neighbors(i).size()) > max_u) {
            root_node = i;
            max_u = w / (1 + queryG_.get_neighbors(i).size());
        }
    }
    return root_node;
}


pair<int, vector<int>> GraphMatch::get_next_node(BiMap &M, 
                                Graph &queryDAG,
                                Graph &revQueryDAG,
                                unordered_set<int> &expendable_u,
                                Graph &CS,
                                vector<unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv,
                                unordered_map<int,int> &weightArray) {

    //looking for candidates for each expendable node
    int u_weight = INT_MAX;
    int u_min = -1;
    vector<int> u_min_candidates;

    for(auto u : expendable_u) {
        //calculating weight for each u in expendable_u
        //using weight to find a u with min weight from expendable_u. 
        int weight = 0;
        vector<int> u_candidates;
        vector<int> u_parents_ids;
        for (auto p : revQueryDAG.get_neighbors(u)) {
            u_parents_ids.push_back(uv2id[p][M.getValueByKey(p)]);
        }

        //get candidates of u
        for(auto v_id : uv2id[u]) {
            auto v = v_id.first, id = v_id.second;
            //make sure all candidates are not mapped
            if(M.hasValue(v)) continue;
            
            if (any_of(u_parents_ids.begin(), u_parents_ids.end(), 
                        [&](auto id_v_p){
                            return !CS.has_edge_quick(id_v_p, id);
                        })
                        ) {
                continue;
            }
            
            u_candidates.push_back(v);
            weight += weightArray[id];
            if (weight >= u_weight) {
                break;
            }
        } 

        if (u_weight > weight){
            u_weight = weight;
            u_min = u;
            u_min_candidates = u_candidates;
        }
    }

    if(u_min < 0)
        return make_pair(-1, u_min_candidates);

    return make_pair(u_min, u_min_candidates);
}


// XXX: add comments
void GraphMatch::build_init_CS(Graph &initCS, 
                                vector<unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv) {

    vector<int> revTopOrder = queryDAG_.get_reversed_topo_order();
    int root = rootIndex_;
    vector<unordered_set<int>> all_candidate_sets(queryG_.num_nodes());
    for (int i = 0; i < all_candidate_sets.size(); ++i) {
        all_candidate_sets[i] = queryG_.get_candidate_set(i, dataG_);
    }
    // root = -1;
    int initCSNodeIndex = 0;
    for (auto u : revTopOrder) {
        // Each node in u_candidate_set is a node in initCS
        for (auto v : all_candidate_sets[u]) {
            initCS.add_node(initCSNodeIndex);
            uv2id[u][v] = initCSNodeIndex;
            id2uv[initCSNodeIndex] = make_pair(u, v);
            initCSNodeIndex++;
            // Check each u's out edge
            for (auto u_prime : queryDAG_.get_neighbors(u)) {
                for (auto v_prime : all_candidate_sets[u_prime]) {
                    if (dataG_.has_edge_quick(v, v_prime)) {
                        initCS.add_edge(initCSNodeIndex - 1, uv2id[u_prime][v_prime]);
                    }
                }
            }
        }
    }
}

bool GraphMatch::refine_CS_wrapper(Graph &CS, 
                                    vector<unordered_map<int, int>> &uv2id,
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
                            vector<unordered_map<int, int>> &uv2id,
                            unordered_map<int, pair<int, int>> &id2uv,
                            Graph &queryDAG) {
    unordered_set<int> filtered_nodes;
    // Iterate all nodes in reversed topo order
    for (auto u : queryDAG.get_reversed_topo_order()) {
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
            if (v_children_u.size() != queryDAG.get_neighbors(u).size()) {
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
                                    vector<unordered_map<int, int>> &uv2id,
                                    unordered_map<int, pair<int, int>> &id2uv) {
    // Iterate all nodes in queryDAG in reversed topo order.
    // Case 1: If a node u has a child u' that has in_degree > 1. All v in C(u) has w(u, v) = 1
    // Case 2: Else for each v in C(u), sum w(u', v') for each u' and v' in C(u') while v' is v's child.
    // w(u, v) = the minimum sum w(u', v') of all u'.
    for (int u : queryDAG.get_reversed_topo_order()) {
        auto C_u = uv2id[u];
        if (all_of(queryDAG.get_neighbors(u).begin(), queryDAG.get_neighbors(u).end(), 
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


void GraphMatch::build_CS(bool enable_refine) {
    csG_ = Graph(true);
    // FIXME:Pre-store all candidate_set to a variable.
#ifndef NDEBUG
    std::chrono::_V2::system_clock::time_point start, stop;

    start = high_resolution_clock::now();
#endif
    uv2id_ = vector<unordered_map<int, int>>(queryG_.num_nodes());
    build_init_CS(csG_, uv2id_, id2uv_);
#ifndef NDEBUG
    stop = high_resolution_clock::now();
    auto build_cs_time = duration_cast<milliseconds>(stop - start).count();
    printf("build init cs %ld ms (%d/%d); ", build_cs_time, csG_.num_nodes(), csG_.num_edges());
#endif

    if (enable_refine) {
        int direction = 1; // 0: reversed
        int iteration = 1;
        while (true) {
            // csG_.print_adjList();
#ifndef NDEBUG            
            cout << "refine iteration " << iteration << endl;
            cout << "nodes before: " << csG_.num_nonempty_nodes() << endl;
#endif
            if (refine_CS_wrapper(csG_, uv2id_, id2uv_, queryDAG_, direction) == false) {
                break;
            }
#ifndef NDEBUG
            cout << "nodes after: " << csG_.num_nonempty_nodes() << endl;
#endif
            direction = 1 - direction;
            iteration += 1;
        }
    }

#ifndef NDEBUG
    start = high_resolution_clock::now();
#endif
    build_weight_array(weightArray_, queryDAG_, csG_, uv2id_, id2uv_);
#ifndef NDEBUG
    stop = high_resolution_clock::now();
    auto build_weight_array_time = duration_cast<milliseconds>(stop - start).count();
    printf("build weight array %ld ms; ", build_weight_array_time);
#endif
}

// TODO: add tests
void GraphMatch::update_init_CS() {
    vector<int> revTopOrder = queryDAG_.get_reversed_topo_order();
    vector<unordered_set<int>> all_candidate_sets(queryG_.num_nodes());
    for (int i = 0; i < all_candidate_sets.size(); ++i) {
        all_candidate_sets[i] = queryG_.get_candidate_set(i, dataG_);
    }
    int root = rootIndex_;
    // root = -1;
    int CSNodeIndex = csG_.num_nodes();
    for (auto u : revTopOrder) {
        // Each node in u_candidate_set is a node in initCS
        for (auto v : all_candidate_sets[u]) {
            if (uv2id_[u].find(v) == uv2id_[u].end()) {
                csG_.add_node(CSNodeIndex);
                uv2id_[u][v] = CSNodeIndex;
                id2uv_[CSNodeIndex] = make_pair(u, v);
                CSNodeIndex++;
            }
            auto uv = uv2id_[u][v];
            
            // Check each u's out edge
            for (auto u_prime : queryDAG_.get_neighbors(u)) {
                for (auto v_prime : all_candidate_sets[u_prime]) {
                    auto uv_prime = uv2id_[u_prime][v_prime];
                    if (dataG_.has_edge_quick(v, v_prime) && !csG_.has_edge(uv, uv_prime)) {
                        csG_.add_edge(uv, uv_prime);
                    }
                }
            }
        }
    }    
}


void GraphMatch::update_CS(bool enable_refine) {

    // FIXME:Pre-store all candidate_set to a variable.
#ifndef NDEBUG
    std::chrono::_V2::system_clock::time_point start, stop;

    start = high_resolution_clock::now();
#endif
    update_init_CS();
#ifndef NDEBUG
    stop = high_resolution_clock::now();
    auto update_cs_time = duration_cast<milliseconds>(stop - start).count();
    printf("update init cs %ld ms; ", update_cs_time);
#endif

    if (enable_refine) {
        int direction = 1; // 0: reversed
        int iteration = 1;
        while (true) {

            if (refine_CS_wrapper(csG_, uv2id_, id2uv_, queryDAG_, direction) == false) {
                break;
            }

            direction = 1 - direction;
            iteration += 1;
        }
    }
#ifndef NDEBUG
    start = high_resolution_clock::now();
#endif
    build_weight_array(weightArray_, queryDAG_, csG_, uv2id_, id2uv_);
#ifndef NDEBUG
    stop = high_resolution_clock::now();
    auto build_weight_array_time = duration_cast<milliseconds>(stop - start).count();
    printf("build weight array %ld ms; ", build_weight_array_time);
#endif
}


bool GraphMatch::backtrack(BiMap &M,
                            vector<BiMap> &allM_prime, 
                            unordered_set<int> expendable_u,
                            unordered_map<int,int> indegrees,
                            int count) {
    bt_count += 1;

    if (M.size() == queryG_.num_nodes()) {
        allM_prime.emplace_back(M);
        return allM_prime.size() < count;
    } else if (M.size() == 0) {
        int u = rootIndex_;
        for (auto nbr : queryDAG_.get_neighbors(u)) {
            indegrees[nbr] += 1;
            if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                expendable_u.insert(nbr);
            }
        }
        for (int i = 0; i < queryDAG_.num_nodes(); ++i) {
            if (queryDAG_.degree(i) == 0) {
                expendable_u.insert(i);
            }
        }
        auto C_u = uv2id_[u];
        for (auto vi : C_u) {
            int v = vi.first;
            M.update(u, v);
            if (backtrack(M, allM_prime, expendable_u, indegrees, count) == false) return false;
            M.eraseByKey(u);
        }
        for (auto nbr : queryDAG_.get_neighbors(u)) {
            if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                expendable_u.erase(nbr);
            }
            indegrees[nbr] -= 1;
        }
    } else {
        auto u_candidates = get_next_node(M, 
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

        // random_shuffle ( u_candidates.second.begin(), u_candidates.second.end() );
        
        for (auto v : u_candidates.second) {
            if (M.hasValue(v) == false) {
                M.update(u, v);
                if (backtrack(M, allM_prime, expendable_u, indegrees, count) == false) return false;
                M.eraseByKey(u);
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


vector<BiMap> GraphMatch::subgraph_isomorphsim(int count) {
    if (hasSubgraph_ == false) return {};

    BiMap M(queryG_.num_nodes());
    vector<BiMap> allM_prime;
    unordered_set<int> expendable_u;
    unordered_map<int, int> indegrees;
    backtrack(M, allM_prime, expendable_u, indegrees, count);

    if (allM_prime.size() >= count) {
        return vector<BiMap>(allM_prime.begin(), allM_prime.begin() + count);
    } else {
        return allM_prime;
    }

}

// This won't change the rootIndex and the queryDAG
bool GraphMatch::update_data_G(Graph dataG) {
    if (dataG.num_nodes() != dataG_.num_nodes()) return false;
    dataG_ = dataG;
    dataG_.generate_edge_checker();
    update_CS();

    return true;
}

bool GraphMatch::early_check() {
    if (dataG_.num_edges() < queryG_.num_edges()) {
        return false;
    }
    vector<int> queryGDegrees(queryG_.num_nodes());
    for (int i = 0; i < queryG_.num_nodes(); ++i) {
        queryGDegrees[i] = queryG_.degree(i);
    }
    sort(queryGDegrees.begin(), queryGDegrees.end(), greater<int>());
    vector<int> dataGDegrees(dataG_.num_nodes());
    for (int i = 0; i < dataG_.num_nodes(); ++i) {
        dataGDegrees[i] = dataG_.degree(i);
    }
    sort(dataGDegrees.begin(), dataGDegrees.end(), greater<int>());
    for (int i = 0; i < queryGDegrees.size(); i++) {
        if (queryGDegrees[i] > dataGDegrees[i]) {
            return false;				
        }
    }
    return true;
}

} // namespace: subiso