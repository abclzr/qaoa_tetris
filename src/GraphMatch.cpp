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
                                Graph &revQueryDAG,
                                unordered_map<int, int>  &weightArray,
                                Graph &CS,
                                unordered_map<int, unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv) {
    //find all unmapped node that their parents are all mapped
    //1. unmp_nodes = all nodes - mapped nodes
    //2. for n in unmp_nodes, if n has parent in unmp_nodes, remove n and all n's children away from unmp_nodes. 
    //3. select a e_v from unmp_nodes with the min weight.
    //4. select a candidate for e_v
    //5. using a min priority queue to store extendable u. 
    //.  each time M is updated by adding <u,v>, queue can be immediately updated by adding u_children
    //.  every backtrack also need to remove the added extendable u
    vector<int> iterOrder = queryDAG.get_topo_order();
    set<int> mapped_u;
    for (auto it : M) mapped_u.insert(it.second);
    
    set<int> unmp_nodes;
    for (auto u : iterOrder) {
        if (mapped_u.find(u) == mapped_u.end()) {
            unmp_nodes.insert(u);
        }
    }

    /* using set difference to find unmapped nodes
    vector<int> unmp_nodes(iterOrder.size());
    vector<int>::iterator it;
    it = std::set_difference (iterOrder.begin(), iterOrder.end(), mapped_u.begin(), mapped_u.end(), unmp_nodes.begin());
    unmp_nodes.resize(it-unmp_nodes.begin());
    */
    set<int> removed_nodes;
    for (auto u : unmp_nodes) {
        auto u_children = queryDAG.get_neighbors(u);
        auto u_parents = revQueryDAG.get_neighbors(u);

        for (auto child : u_children) {
            removed_nodes.insert(child);
        }

        if (removed_nodes.find(u) != removed_nodes.end()){
            continue;
        }
        for (auto p : u_parents){
            if(mapped_u.find(p) == mapped_u.end()){
                removed_nodes.insert(u);
            }
        }
    }

    set<int> expendable_u = unmp_nodes;
    
    for(auto u : removed_nodes) {
        expendable_u.erase(u);
    }

    if(expendable_u.size() == 0) {
        cout<<"expendable_u is empty."<<endl;
        assert(false);
    }

    //using weight to find a u from expendable_u. 
    int u_weight = INT_MAX;
    int u_min = -1;
    for(auto u : expendable_u) {
        int weight = 0;
        for(auto v_id : uv2id[u]){
            weight += weightArray[v_id.second];
        }
        if (u_weight > weight){
            u_weight = weight;
            u_min = u;
        }
    }

    if(u_min < 0)
        assert(false);

    return u_min;
}

set<int> set_intersec(set<int> s1, set<int> s2){
    std::vector<int> v(10);
    std::vector<int>::iterator it;

    it=std::set_intersection (s1.begin(), s1.end(), s2.begin(), s2.end(), v.begin());
    v.resize(it-v.begin());

    set<int> intersection(v.begin(),v.end());
    return intersection;
}

set<int> GraphMatch::get_expendable_candidates(unordered_map<int, int> &M_prime, int expend_u, 
                                Graph &revQueryDAG,
                                Graph &CS,
                                unordered_map<int, unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv) {
    //1. get u's parent
    //2. find intersection of u's parent_id's children
    //case1: find intersection, continue backtrack
    //case2: did not find intersection, unmap the last u. 
    auto u_parents = revQueryDAG.get_neighbors(expend_u);

    set<int> expendable_id;
    for(auto p : u_parents) {
        int v_p = M_prime.at(p); //this will throw an exception if p not mapped.
        int id_v_p = uv2id[p][v_p];
        auto id_neigh_p_cs = CS.get_neighbors(id_v_p);
        
        if(expendable_id.size() == 0){
            expendable_id = id_neigh_p_cs;
        }else{
            expendable_id = set_intersec(expendable_id,id_neigh_p_cs);
        }
    }
    //convert id to v
    set<int> expendable_v;
    for(auto id : expendable_id) {
        expendable_v.insert(id2uv[id].second);
    }
    // get the intersection of currently found expendable_v and candidate set of expend_u
    set<int> candidates = CS.get_candidate_set(expend_u, CS);

    return set_intersec(expendable_v, candidates);
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
                }) || 
            u_children.size() == 0) { // Case 1
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
        int u = get_next_node(M, queryDAG_, revQueryDAG_, weightArray_, 
                        csG_, uv2id_, id2uv_);
        set<int> EC_u = get_expendable_candidates(M_prime, u, 
                                revQueryDAG_, csG_, uv2id_, id2uv_);

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