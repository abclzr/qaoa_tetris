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

    pair<int, vector<int>> GraphMatch::get_next_node1(BiMap &M,
                                                           Graph &queryDAG,
                                                           Graph &revQueryDAG,
                                                           unordered_map<int, pair<int, unordered_set<int>>> &expandable_u,
                                                           int lastExpandU,
                                                           int prevExpandV,
                                                           int curExpandV,
                                                           Graph &CS,
                                                           vector<unordered_map<int, int>> &uv2id,
                                                           unordered_map<int, uint32_t> &id2uv,
                                                           vector<int> &weightArray) {
        // #ifndef NDEBUG
        //         printf("\nget_next_node lastU %d (%d), prevV %d, curV %d; candidates: ", lastExpandU, M.getValueByKey(lastExpandU), prevExpandV, curExpandV);
        //         for (auto it : expandable_u) {
        //             printf("%d, ", it.first);
        //         }
        //         printf("\n");
        // #endif
        for (auto u_it = expandable_u.begin(); u_it != expandable_u.end(); ++u_it) {
            auto u = u_it->first;
            auto u_weight = &u_it->second.first;
            auto u_candidates = &u_it->second.second;
            // #ifndef NDEBUG
            //             printf("\tBefore expnadable_u: u %d, u_weight %d, candidates: ", u, *u_weight);
            //             for (auto vv : *u_candidates) {
            //                 printf("%d, ", vv);
            //             }
            //             printf("\n");
            // #endif
            vector<int> u_parents_ids;
            for (auto p : revQueryDAG.get_neighbors(u)) {
                u_parents_ids.push_back(uv2id[p][M.getValueByKey(p)]);
            }

            if (queryDAG_.has_edge(lastExpandU, u)) {
                if (prevExpandV == -1) {
                    u_candidates->clear();
                    *u_weight = 0;
                    for (auto v_id : uv2id[u]) {
                        // add
                        auto v = v_id.first, id = v_id.second;
                        // make sure all candidates are not mapped
                        if (M.hasValue(v))
                            continue;

                        if (any_of(u_parents_ids.begin(), u_parents_ids.end(),
                                   [&](auto id_v_p) {
                                       return !CS.has_edge_quick(id_v_p, id);
                                   })) {
                            continue;
                        }
                        u_candidates->insert(v);
                        *u_weight += weightArray[id];
                    }
                } else {
                    auto prevExpandV_id = uv2id[lastExpandU][prevExpandV];
                    auto curExpandV_id = uv2id[lastExpandU][curExpandV];
                    for (auto v_id : uv2id[u]) {
                        auto v = v_id.first, id = v_id.second;
                        if (csG_.has_edge_quick(curExpandV_id, id) == false &&
                            csG_.has_edge_quick(prevExpandV_id, id) == true) {
                            // remove
                            if (u_candidates->find(v) != u_candidates->end()) {
                                u_candidates->erase(v);
                                *u_weight -= weightArray[id];
                            }
                        } else if (csG_.has_edge_quick(curExpandV_id, id) == true &&
                                   csG_.has_edge_quick(prevExpandV_id, id) == false) {
                            // add
                            if (!M.hasValue(v)) {
                                if (all_of(u_parents_ids.begin(), u_parents_ids.end(),
                                           [&](auto id_v_p) {
                                               return CS.has_edge_quick(id_v_p, id);
                                           })) {
                                    u_candidates->insert(v);
                                    *u_weight += weightArray[id];
                                }
                            }
                        }
                    }
                }
            } else {
                // remove
                if (u_candidates->find(curExpandV) != u_candidates->end()) {
                    u_candidates->erase(curExpandV);
                    *u_weight -= weightArray[uv2id[u][curExpandV]];
                }
                // add prevExpandV
                if (!M.hasValue(prevExpandV) && uv2id[u].find(prevExpandV) != uv2id[u].end()) {
                    auto prevExpandV_id = uv2id[u][prevExpandV];
                    if (all_of(u_parents_ids.begin(), u_parents_ids.end(),
                               [&](auto id_v_p) {
                                   return CS.has_edge_quick(id_v_p, prevExpandV_id);
                               })) {
                        u_candidates->insert(prevExpandV);
                        *u_weight += weightArray[prevExpandV_id];
                    }
                }
            }
            // #ifndef NDEBUG
            //             printf("\tAfter expnadable_u: u %d, u_weight %d, candidates: ", u, *u_weight);
            //             for (auto vv : *u_candidates) {
            //                 printf("%d, ", vv);
            //             }
            //             printf("\n");
            // #endif
        }

        int u_weight = INT_MAX;
        int u_min = -1;
        vector<int> u_min_candidates;
        for (auto u_it : expandable_u) {
            if (u_it.second.second.size() == 0)
                return make_pair(-1, u_min_candidates);
            auto u = u_it.first;
            auto weight = u_it.second.first;
            if (u_weight > weight) {
                u_weight = weight;
                u_min = u;
            }
        }
        // #ifndef NDEBUG
        //         printf("select u_min %d\n", u_min);
        // #endif
        for (auto v : expandable_u[u_min].second) {
            u_min_candidates.push_back(v);
        }

        return make_pair(u_min, u_min_candidates);
    }

    pair<int, bitset<32>> GraphMatch::get_next_node(BiMap &M, unordered_set<int> &expandable_u) {

        // looking for candidates for each expandable node
        int u_weight = INT_MAX;
        int u_min = -1;
        bitset<32> u_min_candidates;

        for (auto u : expandable_u) {
            // calculating weight for each u in expandable_u
            // using weight to find a u with min weight from expandable_u.
            int weight = 0;
            bitset<32> u_candidates;
            vector<int> u_parents_ids;
            for (auto p : revQueryDAG_.get_neighbors(u)) {
                u_parents_ids.push_back(uv2id_[p][M.getValueByKey(p)]);
            }

            // get candidates of u
            for (auto v_id : uv2id_[u]) {
                auto v = v_id.first, id = v_id.second;
                // make sure all candidates are not mapped
                if (M.hasValue(v))
                    continue;

                if (any_of(u_parents_ids.begin(), u_parents_ids.end(),
                           [&](auto id_v_p) {
                               return !csG_.has_edge_quick(id_v_p, id);
                           })) {
                    continue;
                }

                u_candidates.set(v);
                weight += weightArray_[id];
                if (weight >= u_weight) {
                    break;
                }
            }

            if (u_candidates.count() == 0) {
                return make_pair(-1, u_candidates);
            }

            if (u_weight > weight) {
                u_weight = weight;
                u_min = u;
                u_min_candidates = u_candidates;
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
    bool combo_check(const T& c, int k, const vector<bitset<32>> &candidates) {
        int n = c.size();
        int combo = (1 << k) - 1;       // k bit sets
        while (combo < 1<<n) {
            
            // checking part
            bitset<32> bs;
            for (int i = 0; i < n; ++i) {
                if ((combo >> i) & 1)
                    // cout << c[i] << ' ';
                    bs |= candidates[c[i]];
            }
            // cout << endl;
            if (bs.count() < k) {
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

    bool GraphMatch::backtrack(BiMap &M,
                               vector<BiMap> &allM_prime,
                               unordered_set<int> expandable_u,
                               vector<int> &indegrees,
                               int count) {
        bt_count += 1;

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
            for (auto vi : C_u) {
                int v = vi.first;
                M.update(u, v);
                if (backtrack(M, allM_prime, expandable_u, indegrees, count) == false)
                    return false;
                M.eraseByKey(u);
            }
            for (auto nbr : queryDAG_.get_neighbors(u)) {
                if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                    expandable_u.erase(nbr);
                }
                indegrees[nbr] -= 1;
            }
        } else {
            auto u_candidates = get_next_node(M, expandable_u);

            int u = u_candidates.first;
            auto v_candidates = u_candidates.second;
            // printf("select %d\ncandidates: ", u);
            // for (auto v : v_candidates) {
            //     printf("%d, ", v);
            // }
            // printf("\n");
            if (u != -1 && v_candidates.count() > 0) {
                expandable_u.erase(u);
/*
                // Here we get candidates for all other nodes
                vector<bitset<32>> candidates(queryG_.num_nodes());
                vector<int> candidates_ids;
                for (int uu = 0; uu < queryG_.num_nodes(); ++uu) {
                    if (M.hasKey(uu)) continue;
                    if (expandable_u.find(uu) != expandable_u.end()) continue;

                    vector<int> u_parents_ids;
                    for (auto p : revQueryDAG_.get_neighbors(uu)) {
                        if (!M.hasKey(p)) continue;
                        u_parents_ids.push_back(uv2id_[p][M.getValueByKey(p)]);
                    }

                    // get candidates of uu
                    for (auto v_id : uv2id_[uu]) {
                        auto v = v_id.first, id = v_id.second;
                        // make sure all candidates are not mapped
                        if (M.hasValue(v)) continue;
                        if (any_of(u_parents_ids.begin(), u_parents_ids.end(),
                           [&](auto id_v_p) {
                               return !csG_.has_edge_quick(id_v_p, id);
                           })) {
                            continue;
                        }
                        candidates[uu].set(v);
                    }

                    if (candidates[uu].count() == 0) { 
                        // printf("quit\n"); 
                        // printf("=====\n");
                        return true; 
                    } else {
                        candidates_ids.push_back(uu);
                    }
                }
                // for (auto id : candidates_ids) {
                //     cout << id << " " << candidates[id] << endl;
                // }
                // (Hall’s Marriage Theorem). Let G = (L, R, E) be a bipartite graph with |L| = |R|.
                // Suppose that for every S ⊆ L, we have |Γ(S)| ≥ |S|. Then G has a perfect matching.
                int maxK = 3;
                maxK = maxK <= candidates_ids.size() ? maxK : candidates_ids.size();
                for (int k = 1; k <= maxK; ++k) {
                    // select k non-empty candidates
                    if (combo_check(candidates_ids, k, candidates) == false) {
                        // printf("quit\n"); 
                        // printf("=====\n");
                        return true;
                    }
                }
                // printf("=====\n");
*/                
                // update in_degree & expandable_u
                for (auto nbr : queryDAG_.get_neighbors(u)) {
                    indegrees[nbr] += 1;
                    if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                        expandable_u.insert(nbr);
                    }
                }

                // random_shuffle ( u_candidates.second.begin(), u_candidates.second.end() );

                // for (auto v : v_candidates) {
                for (auto v = v_candidates._Find_first(); v < v_candidates.size(); v = v_candidates._Find_next(v)) {
                    // printf("v %d\n", v);
                    if (M.hasValue(v) == false) {
                        M.update(u, v);
                        if (backtrack(M, allM_prime, expandable_u, indegrees, count) == false)
                            return false;
                        M.eraseByKey(u);
                    }
                }

                for (auto nbr : queryDAG_.get_neighbors(u)) {
                    if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                        expandable_u.erase(nbr);
                    }
                    indegrees[nbr] -= 1;
                }
            }
        }
        return true;
    }

    bool GraphMatch::backtrack1(BiMap &M,
                                     vector<BiMap> &allM_prime,
                                     unordered_map<int, pair<int, unordered_set<int>>> &expandable_u,
                                     int lastExpandU,
                                     int prevExpandV,
                                     int curExpandV,
                                     unordered_map<int, int> indegrees,
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
                    expandable_u[nbr] = make_pair(0, unordered_set<int>());
                }
            }
            for (int i = 0; i < queryDAG_.num_nodes(); ++i) {
                if (queryDAG_.degree(i) == 0) {
                    expandable_u[i] = make_pair(0, unordered_set<int>());
                }
            }
            auto C_u = uv2id_[u];
            lastExpandU = u;
            prevExpandV = -1;
            // printf("root: %d vs: ", u);
            // for (auto vi : C_u) {
            //     printf("%d ", vi.first);
            // }
            // printf("\n");
            for (auto vi : C_u) {
                int v = vi.first;
                curExpandV = v;
                M.update(u, v);
                if (backtrack1(M,
                                    allM_prime,
                                    expandable_u,
                                    lastExpandU,
                                    prevExpandV,
                                    curExpandV,
                                    indegrees,
                                    count) == false)
                    return false;
                M.eraseByKey(u);
                prevExpandV = v;
            }
            for (auto nbr : queryDAG_.get_neighbors(u)) {
                if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                    expandable_u.erase(nbr);
                }
                indegrees[nbr] -= 1;
            }
        } else {
            auto u_candidates = get_next_node1(M,
                                                    queryDAG_,
                                                    revQueryDAG_,
                                                    expandable_u,
                                                    lastExpandU,
                                                    prevExpandV,
                                                    curExpandV,
                                                    csG_,
                                                    uv2id_,
                                                    id2uv_,
                                                    weightArray_);

            auto expandable_u_ = expandable_u;

            int u = u_candidates.first;
            auto v_candidates = u_candidates.second;
            if (u != -1 && v_candidates.size() > 0) {
                expandable_u.erase(u);

                // update in_degree & expandable_u
                for (auto nbr : queryDAG_.get_neighbors(u)) {
                    indegrees[nbr] += 1;
                    if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                        expandable_u[nbr] = make_pair(0, unordered_set<int>());
                    }
                }

                // random_shuffle ( u_candidates.second.begin(), u_candidates.second.end() );
                lastExpandU = u;
                prevExpandV = -1;
                for (auto v : u_candidates.second) {
                    curExpandV = v;
                    if (M.hasValue(v) == false) {
                        M.update(u, v);
                        if (backtrack1(M,
                                            allM_prime,
                                            expandable_u,
                                            lastExpandU,
                                            prevExpandV,
                                            curExpandV,
                                            indegrees,
                                            count) == false)
                            return false;
                        M.eraseByKey(u);
                        prevExpandV = v;
                    }
                }

                for (auto nbr : queryDAG_.get_neighbors(u)) {
                    if (indegrees[nbr] == queryDAG_.in_degree(nbr)) {
                        expandable_u.erase(nbr);
                    }
                    indegrees[nbr] -= 1;
                }

                expandable_u = expandable_u_;
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
        backtrack(M, allM_prime, expandable_u, indegrees, count);
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