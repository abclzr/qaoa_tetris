#include "GraphMatch.hpp"
#include "BiMap.hpp"

#include <algorithm>
#include <chrono>
#include <unordered_map>

using namespace std;
using namespace std::chrono;
using namespace qaoagraph;
using namespace bimap;

namespace subiso {

    int GraphMatch::get_root_node(Graph graph) {
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
        vector<int> degrees(graph.num_nodes());
        for (int i = 0; i < graph.num_nodes(); ++i) {
            degrees[i] = graph.degree(i);
        }
        float max_u = -1;
        for (int i = 0; i < graph.num_nodes(); ++i) {
            float w = (float)degrees[i];
            for (auto nbr : graph.get_neighbors(i)) {
                w += (float)degrees[nbr];
            }
            if (w / (1 + graph.get_neighbors(i).size()) > max_u) {
                root_node = i;
                max_u = w / (1 + graph.get_neighbors(i).size());
            }
        }
        return root_node;
    }

    void GraphMatch::update_candidates(BiMap &M, int u, int v, 
                                        vector<uint32_t> &u_candidates) {
        M.update(u, v);              
        auto id = uv2id_[u][v];                      
        for (int i = 0; i < queryG_.num_nodes(); ++i) {
            if (M.hasKey(i)) continue;
            // remove v from i's candidate
            u_candidates[i] &= ~(1 << v);
            // resolve query graph edge constraints
            auto id_nbrs = id2uNbrs_[id][i];
            // printf("i %d id_nbrs %u before %u --- ", i, id_nbrs, u_candidates[i]);
            if (id_nbrs != 0) u_candidates[i] &= id_nbrs;
            // printf("i %d id_nbrs %u after %u\n", i, id_nbrs, u_candidates[i]);
        }
    }

    void GraphMatch::reset_candidates(BiMap &M, int u,
                                        vector<uint32_t> &u_candidates,
                                        const vector<uint32_t> &old_u_candidates) {
        M.eraseByKey(u);                                 
        for (int i = 0; i < queryG_.num_nodes(); ++i) {
            if (M.hasKey(i)) continue;
            u_candidates[i] = old_u_candidates[i];
        }
    }

    pair<int, uint32_t> GraphMatch::get_next_node(BiMap &M, 
                                                    unordered_set<int> &expandable_u,
                                                    vector<uint32_t> &u_candidates) {

        // looking for candidates for each expandable node
        int u_min_weight = INT_MAX;
        int u_min = -1;
        uint32_t u_min_candidates = 0;

        for (auto u : expandable_u) {
            // calculating weight for each u in expandable_u
            // using weight to find a u with min weight from expandable_u.
            int weight = 0;
            uint32_t candidates = u_candidates[u];
            while (candidates != 0) {
                int v = 31 - __builtin_clz(candidates);
                candidates ^= (1 << v);
                weight += weightArray_[uv2id_[u][v]];
                if (weight >= u_min_weight) {
                    break;
                }
            }

            if (u_min_weight > weight) {
                u_min_weight = weight;
                u_min = u;
                u_min_candidates = u_candidates[u];
            }
        }

        return make_pair(u_min, u_min_candidates);
    }

    // XXX: add comments
    void GraphMatch::build_init_CS(Graph &initCS,
                                   vector<unordered_map<int, int>> &uv2id,
                                   unordered_map<int, uint32_t> &id2uv,
                                   int rootCandidate) {

        vector<int> revTopOrder = queryDAG_.get_reversed_topo_order();
        int root = rootIndex_;
        vector<unordered_set<int>> all_candidate_sets(queryG_.num_nodes());
        for (int i = 0; i < all_candidate_sets.size(); ++i) {
            all_candidate_sets[i] = queryG_.get_candidate_set(i, dataG_);
            if (i == root && rootCandidate != -1) all_candidate_sets[i] = {rootCandidate};
        }
        // root = -1;
        int initCSNodeIndex = 0;
        for (auto u : revTopOrder) {
            // Each node in u_candidate_set is a node in initCS
            for (auto v : all_candidate_sets[u]) {
                initCS.add_node(initCSNodeIndex);
                uv2id[u][v] = initCSNodeIndex;
                // id2uv[initCSNodeIndex] = make_pair(u, v);
                id2uv[initCSNodeIndex] = (u << 16) + v;
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
                                       unordered_map<int, uint32_t> &id2uv,
                                       Graph &queryDAG,
                                       int reversed) {
        bool changed = false;
        if (reversed == 0) { // 0: original dag
            changed = refine_CS(CS, uv2id, id2uv, queryDAG);
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
                               unordered_map<int, uint32_t> &id2uv,
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
                    // int v_child_u = id2uv[v_child].first;
                    int v_child_u = id2uv[v_child] >> 16;
                    v_children_u.insert(v_child_u);
                }
                if (v_children_u.size() != queryDAG.get_neighbors(u).size()) {
                    filtered_nodes.insert(id);
                }
            }
        }
        if (filtered_nodes.size() == 0)
            return false;

        // For all the nodes in filtered_nodes, we clean CS, uv2id and id2uv
        CS.remove_nodes(filtered_nodes);
        for (auto id : filtered_nodes) {
            // int u = id2uv[id].first, v = id2uv[id].second;
            int u = id2uv[id] >> 16, v = id2uv[id] & 0xffff;
            id2uv.erase(id);
            uv2id[u].erase(v);
        }

        return true;
    }

    void GraphMatch::build_weight_array(vector<int> &weightArray,
                                        Graph &queryDAG,
                                        Graph &CS,
                                        vector<unordered_map<int, int>> &uv2id,
                                        unordered_map<int, uint32_t> &id2uv) {
        // Iterate all nodes in queryDAG in reversed topo order.
        // Case 1: If a node u has a child u' that has in_degree > 1. All v in C(u) has w(u, v) = 1
        // Case 2: Else for each v in C(u), sum w(u', v') for each u' and v' in C(u') while v' is v's child.
        // w(u, v) = the minimum sum w(u', v') of all u'.
        for (int u : queryDAG.get_reversed_topo_order()) {
            auto C_u = uv2id[u];
            if (all_of(queryDAG.get_neighbors(u).begin(), queryDAG.get_neighbors(u).end(),
                       [&](int u_prime) {
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
                        int u_prime = id2uv[id_prime] >> 16;
                        weights[u_prime] += weightArray[id_prime];
                    }
                    weightArray[id] = (*min_element(weights.begin(), weights.end(),
                                                    [](const auto &lhs, const auto &rhs) { return lhs.second < rhs.second; }))
                                          .second;
                }
            }
        }
    }

    void GraphMatch::build_CS(bool enable_refine, int rootCandidate) {
        csG_ = Graph(true);
        // FIXME:Pre-store all candidate_set to a variable.
#ifndef NDEBUG
        std::chrono::_V2::system_clock::time_point start, stop;

        start = high_resolution_clock::now();
#endif
        uv2id_ = vector<unordered_map<int, int>>(queryG_.num_nodes());
        build_init_CS(csG_, uv2id_, id2uv_, rootCandidate);
#ifndef NDEBUG
        stop = high_resolution_clock::now();
        auto build_cs_time = duration_cast<milliseconds>(stop - start).count();
        printf("\tbuild init cs %ld ms (%d/%d);\n", build_cs_time, csG_.num_nodes(), csG_.num_edges());
        fflush(stdout);
#endif

        if (enable_refine) {
            int direction = 1; // 0: reversed
            int iteration = 1;
            while (true) {
                // csG_.print_adjList();
#ifndef NDEBUG
                printf("\trefine iteration %d before %d/%d\n", iteration, csG_.num_nonempty_nodes(), csG_.num_edges());
#endif
                if (refine_CS_wrapper(csG_, uv2id_, id2uv_, queryDAG_, direction) == false) {
                    break;
                }
#ifndef NDEBUG
                printf("\trefine iteration %d after %d/%d\n", iteration, csG_.num_nonempty_nodes(), csG_.num_edges());
#endif
                direction = 1 - direction;
                iteration += 1;
            }
        }

        // for (int i = 0; i < queryDAG_.num_nodes(); ++i) {
        //     printf("u %d: ", i);
        //     for (auto u : uv2id_[i]) {
        //         printf("[%d, %d]; ", u.first, u.second);
        //     }
        //     printf("\n");
        // }
        // csG_.print_adjList();

        // build id2uNbrs_
        id2uNbrs_ = vector<vector<uint32_t>>(csG_.num_nodes(), vector<uint32_t>(dataG_.num_nodes(), 0));
        for (auto edge : csG_.get_edges()) {
            auto id1 = edge[0], id2 = edge[1];
            int u2 = id2uv_[id2] >> 16;
            int v2 = id2uv_[id2] & 0xffff;
            id2uNbrs_[id1][u2] |= (1 << v2);
        }
        // for (int i = 0; i < csG_.num_nodes(); ++i) {
        //     for (int j = 0; j < queryG_.num_nodes(); ++j) {
        //         printf("id: %d u: %d str: %u | ",i ,j, id2uNbrs_[i][j]);
        //         auto candidates_ = id2uNbrs_[i][j];
        //         while (candidates_ != 0) {
        //             int v = 31 - __builtin_clz(candidates_);
        //             candidates_ ^= (1 << v);
        //             printf("%d, ", v);
        //         }
        //         printf("\n");
        //     }
        // }

#ifndef NDEBUG
        start = high_resolution_clock::now();
#endif
        weightArray_ = vector<int>(csG_.num_nodes(), 0);
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
                    // id2uv_[CSNodeIndex] = make_pair(u, v);
                    id2uv_[CSNodeIndex] = (u << 16) + v;
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

    template<typename T>
    inline bool combo_check(const T& c, int k, const vector<uint32_t> &candidates) {
        int n = c.size();
        int combo = (1 << k) - 1;       // k bit sets
        while (combo < 1<<n) {
            
            // checking part
            uint32_t bs = 0;
            for (int i = 0; i < n; ++i) {
                if ((combo >> i) & 1)
                    // cout << c[i] << ' ';
                    bs |= candidates[c[i]];
            }
            // cout << endl;
            if (__builtin_popcount(bs) < k) {
                return false;
            }

            int x = combo & -combo;
            int y = combo + x;
            int z = (combo & ~y);
            combo = z / x;
            combo >>= 1;
            combo |= y;
        }
        return true;
    }

    inline bool early_backtrack_termination_check(BiMap &M, vector<uint32_t> &u_candidates, int maxK) {
        uint32_t check_ones = 0;
        vector<int> candidates_ids;
        for (int uu = 0; uu < u_candidates.size(); ++uu) {
            if (M.hasKey(uu)) continue;
            // if (expandable_u.find(uu) != expandable_u.end()) continue;
            if (u_candidates[uu] == 0) {
                return true;
            }
            else if (__builtin_popcount(u_candidates[uu]) == 1) {
                int k = 31 - __builtin_clz(u_candidates[uu]);
                if (((check_ones & (1 << k)) >> k)) { // k-th bit is one
                    return true;
                } else {
                    check_ones |= (1 << k);
                    candidates_ids.push_back(uu);
                }
            } 
            else {
                if ((u_candidates[uu] & (~check_ones)) == 0) {
                    return true;
                }
                if (__builtin_popcount(u_candidates[uu]) < maxK) {
                    candidates_ids.push_back(uu);
                }
            }
        }

        // (Hall’s Marriage Theorem). Let G = (L, R, E) be a bipartite graph with |L| = |R|.
        // Suppose that for every S ⊆ L, we have |Γ(S)| ≥ |S|. Then G has a perfect matching.
        maxK = maxK <= candidates_ids.size() ? maxK : candidates_ids.size();
        for (int k = 3; k <= maxK; ++k) {
            // select k non-empty candidates
            if (combo_check(candidates_ids, k, u_candidates) == false) {
                return true;
            }
        }

        return false;
    }

    bool GraphMatch::backtrack(BiMap &M,
                               vector<BiMap> &allM_prime,
                               unordered_set<int> &expandable_u,
                               vector<uint32_t> &u_candidates,
                               vector<int> &indegrees,
                               int count) {
        bt_count += 1;

        // printf("expandable: ");
        // for (auto it : expandable_u) {
        //     printf("%d ", it);
        // }
        // printf("\n");

        if (M.size() == queryG_.num_nodes()) {
            allM_prime.emplace_back(M);
            return allM_prime.size() < count;
        } else if (M.size() == 0) {
            int u = rootIndex_;
            for (auto nbr : queryDAG_.get_neighbors(u)) {
                indegrees[nbr] += 1;
                if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                    expandable_u.insert(nbr);
                }
            }
            for (int i = 0; i < queryDAG_.num_nodes(); ++i) {
                if (queryDAG_.degree(i) == 0) {
                    expandable_u.insert(i);
                }
            }
            auto C_u = uv2id_[u];
            // printf("root: %d vs: ", u);
            // for (auto vi : C_u) {printf("%d ", vi.first);}
            // printf("\n");
            // for (int i = 0; i < u_candidates.size(); ++i) {
            //     printf("node %d candidates str: %u | ", i, u_candidates[i]);
            //     auto candidates_ = u_candidates[i];
            //     while (candidates_ != 0) {
            //         int v = 31 - __builtin_clz(candidates_);
            //         candidates_ ^= (1 << v);
            //         printf("%d, ", v);
            //     }
            //     printf("\n");
            // }

            for (auto vi : C_u) {
                int v = vi.first;
                // M.update(u, v);
                auto old_u_candidates = u_candidates;
                update_candidates(M, u, v, u_candidates);

                if (backtrack(M, allM_prime, expandable_u, u_candidates, indegrees, count) == false)
                    return false;
                
                // M.eraseByKey(u);
                reset_candidates(M, u, u_candidates, old_u_candidates);
            }
            for (auto nbr : queryDAG_.get_neighbors(u)) {
                if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                    expandable_u.erase(nbr);
                }
                indegrees[nbr] -= 1;
            }
        } else {
            auto next_node = get_next_node(M, expandable_u, u_candidates);

            int u = next_node.first;
            auto candidates = next_node.second;

            // printf("select: %d candidates str: %u | ", u, candidates);
            // auto candidates_ = candidates;
            // while (candidates_ != 0) {
            //     int v = 31 - __builtin_clz(candidates_);
            //     candidates_ ^= (1 << v);
            //     printf("%d, ", v);
            // }
            // printf("\n");

            if (u != -1 && candidates != 0) {
                expandable_u.erase(u);
            
                // update in_degree & expandable_u
                for (auto nbr : queryDAG_.get_neighbors(u)) {
                    indegrees[nbr] += 1;
                    if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                        expandable_u.insert(nbr);
                    }
                }

                while (candidates != 0) {
                    int v = 31 - __builtin_clz(candidates);
                    candidates ^= (1 << v);
                    // printf("v %d\n", v);
                    if (M.hasValue(v) == false) {
                        // M.update(u, v);
                        auto old_u_candidates = u_candidates;
                        update_candidates(M, u, v, u_candidates);
                        // for (int i = 0; i < u_candidates.size(); ++i) {
                        //     printf("node %d candidates str: %u | ", i, u_candidates[i]);
                        //     auto candidates_ = u_candidates[i];
                        //     while (candidates_ != 0) {
                        //         int v = 31 - __builtin_clz(candidates_);
                        //         candidates_ ^= (1 << v);
                        //         printf("%d, ", v);
                        //     }
                        //     printf("\n");
                        // }

                        if (early_backtrack_termination_check(M, u_candidates, maxK_) == false && 
                            backtrack(M, allM_prime, expandable_u, u_candidates, indegrees, count) == false)
                            return false;

                        // M.eraseByKey(u);
                        reset_candidates(M, u, u_candidates, old_u_candidates);
                    }
                }
                

                for (auto nbr : queryDAG_.get_neighbors(u)) {
                    if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                        expandable_u.erase(nbr);
                    }
                    indegrees[nbr] -= 1;
                }

                expandable_u.insert(u);

            }
        }
        return true;
    }


    vector<BiMap> GraphMatch::subgraph_isomorphsim(int count) {
        if (has_subgraph() == false)
            return {};

        BiMap M(dataG_.num_nodes());
        vector<BiMap> allM_prime;
        vector<int> indegrees(queryG_.num_nodes(), 0);

        unordered_set<int> expandable_u;
        vector<uint32_t> u_candidates(queryG_.num_nodes(), 0);
        for (int i = 0; i < queryG_.num_nodes(); ++i) {
            for (auto v_id : uv2id_[i]) {
                int v = v_id.first;
                u_candidates[i] |= (1 << v);
            }
        }
        backtrack(M, allM_prime, expandable_u, u_candidates, indegrees, count);
        // unordered_map<int, pair<int, unordered_set<int>>> expandable_u;
        // backtrack1(M, allM_prime, expandable_u, -1, -1, -1, indegrees, count);

        if (allM_prime.size() >= count) {
            return vector<BiMap>(allM_prime.begin(), allM_prime.begin() + count);
        } else {
            return allM_prime;
        }
    }

    // This won't change the rootIndex and the queryDAG
    bool GraphMatch::update_data_G(Graph dataG) {
        if (dataG.num_nodes() != dataG_.num_nodes())
            return false;
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