#include "Graph.hpp"
#include <algorithm>
#include <iostream>
#include <queue>

using namespace std;

namespace qaoagraph {

bool Graph::add_node(int u) {
    if (u < 0)
        return false;
    if (u >= numNodes_) {
        numNodes_ = u + 1;
        adjList_.resize(numNodes_);
        in_degree_.resize(numNodes_, 0);
        out_degree_.resize(numNodes_, 0);
    }
    topo_order_need_update_ = true;
    return true;
}

bool Graph::add_edge(int u, int v) {
    if (u < 0 || v < 0)
        return false;
    add_node(u);
    add_node(v);
    if (adjList_[u].find(v) == adjList_[u].end()) {
        adjList_[u].insert(v);
        numEdges_ += 1;
        out_degree_[u] += 1;
        in_degree_[v] += 1;
    }
    if (!directed_ && adjList_[v].find(u) == adjList_[v].end()) {
        adjList_[v].insert(u);
        out_degree_[v] += 1;
        in_degree_[u] += 1;
    }
    topo_order_need_update_ = true;
    return true;
}

void Graph::remove_nodes(unordered_set<int> nodes) {
    // XXX: we are actually removing edges related to the node but
    // not remove nodes.
    topo_order_need_update_ = true;
    for (int i = 0; i < numNodes_; i++) {
        if (nodes.find(i) != nodes.end()) {
            numEdges_ -= adjList_[i].size();
            out_degree_[i] -= adjList_[i].size();
            for (auto node : adjList_[i]) {
                in_degree_[node] -= 1;
            }
            adjList_[i] = unordered_set<int>();
        } else {
            unordered_set<int> i_nbrs = adjList_[i];
            for (auto u : i_nbrs) {
                if (nodes.find(u) != nodes.end()) {
                    adjList_[i].erase(u);
                    numEdges_ -= 1;
                    in_degree_[u] -= 1;
                    out_degree_[i] -= 1;
                }
            }
        }
    }
}

vector<vector<int>> Graph::get_edges() {
    vector<vector<int>> edges;
    for (int u = 0; u < numNodes_; u++) {
        for (auto vi = adjList_[u].begin(); vi != adjList_[u].end(); ++vi) {
            edges.push_back({u, *vi});
        }
    }
    return edges;
}

int Graph::in_degree(int u) {
    return in_degree_[u];
}

int Graph::out_degree(int u) {
    return out_degree_[u];
}

int Graph::degree(int u) {
    if (!directed_)
        return out_degree(u);
    else
        return out_degree(u) + in_degree(u);
}

bool Graph::load_from_file(ifstream &graphFile, bool directed) {
    directed_ = directed;
    int numEdges;
    graphFile >> numNodes_ >> numEdges;
    adjList_.resize(numNodes_);
    in_degree_.resize(numNodes_, 0);
    out_degree_.resize(numNodes_, 0);

    int n1, n2;
    while (graphFile >> n1 >> n2) {
        if (n1 >= numNodes_ || n2 >= numNodes_) {
            return false;
        }
        if (!add_edge(n1, n2)) {
            return false;
        }
    }
    if (numEdges_ != numEdges) {
        return false;
    }

    return true;
}

// XXX: nodes at the same should have an order. E.g., in fig. 5, u6 is pointed to u7.
Graph Graph::generate_dag(int u) {
    Graph g(numNodes_, true);
    vector<int> visited(numNodes_, 0);
    queue<int> q;
    q.emplace(u);
    visited[u] = 1;

    while (!q.empty()) {
        u = q.front();
        q.pop();
        for (auto vi : adjList_[u]) {
            if (visited[vi] == 0) {
                visited[vi] = 1;
                q.emplace(vi);
                g.add_edge(u, vi);
            } else if (visited[vi] == 1) {
                g.add_edge(u, vi);
            } else { // visited[*vi] == 2
                continue;
            }
        }
        visited[u] = 2;
    }

    return g;
}

Graph Graph::generate_reversed_graph() {
    Graph g(numNodes_, true);
    for (auto edge : get_edges()) {
        g.add_edge(edge[1], edge[0]);
    }
    return g;
}

int Graph::num_nonempty_nodes() {
    int nonempty_nodes = 0;
    for (int i = 0; i < numNodes_; ++i) {
        nonempty_nodes += adjList_[i].empty() ? 0 : 1;
    }
    return nonempty_nodes;
}

void Graph::topo_sort_util(int u, vector<bool> &visited, vector<int> &topo_order) {
    // Mark the current node as visited.
    visited[u] = true;

    // Recur for all the vertices
    // adjacent to this vertex
    for (auto vi : adjList_[u])
        if (!visited[vi])
            topo_sort_util(vi, visited, topo_order);

    // Push current vertex to stack
    // which stores result
    topo_order.insert(topo_order.begin(), u);
}

void Graph::update_topo_order() {
    vector<bool> visited(numNodes_, false);

    for (int i = 0; i < numNodes_; i++) {
        if (!visited[i]) {
            topo_sort_util(i, visited, topo_order_);
        }
    }
    rev_topo_order_ = vector<int>(topo_order_.rbegin(), topo_order_.rend());
    topo_order_need_update_ = false;
}

vector<int> &Graph::get_topo_order() {
    if (topo_order_need_update_ == true) {
        update_topo_order();
    }
    return topo_order_;
}

vector<int> &Graph::get_reversed_topo_order() {
    if (topo_order_need_update_ == true) {
        update_topo_order();
    }
    return rev_topo_order_;
}

unordered_set<int> Graph::get_candidate_set(int u, Graph &g) {
    int u_degree = degree(u);
    unordered_set<int> candidate_set;
    for (int i = 0; i < g.num_nodes(); i++) {
        if (u_degree <= g.degree(i)) {
            candidate_set.insert(i);
        }
    }
    return candidate_set;
}

// Find simple cylces in a directed graph. Follow networkx simple_cycle implementation,
// which is an iterative version of Johnson's algorithm
// https://github.com/networkx/networkx/blob/main/networkx/algorithms/cycles.py
// [1] Finding all the elementary circuits of a directed graph.
vector<vector<int>> Graph::simple_cycles() {
    vector<vector<int>> allCycles;

    auto unblock_ = [] (int thisnode, unordered_set<int> &blocked, unordered_map<int, unordered_set<int>> &B) {
        unordered_set<int> st_;
        st_.insert(thisnode);
        while (!st_.empty()) {
            int node = *st_.begin();
            st_.erase(node);
            if (blocked.find(node) != blocked.end()) {
                blocked.erase(node);
                for (auto n : B[node]) {
                    st_.insert(n);
                }
                B[node].clear();
            }
        }
    };

    auto sccs = strongly_connected_components();

    while (!sccs.empty()) {
        auto scc = sccs[0];
        sccs.erase(sccs.begin());
        auto subgraph = generate_subgraph(scc);
        // nodes in Johnson's algorithm need an order.
        int startnode = *scc.begin();
        scc.erase(startnode);
        vector<int> path = {startnode};
        unordered_set<int> blocked;
        unordered_set<int> closed;
        blocked.insert(startnode);
        unordered_map<int, unordered_set<int>> B;
        queue<int> startnodeNbrs;
        for (auto nbr : subgraph.get_neighbors(startnode)) {
            startnodeNbrs.push(nbr);
        }
        stack<pair<int, queue<int>>> st;
        st.push(make_pair(startnode, startnodeNbrs));
        while (!st.empty()) {
            auto top = &st.top();
            int thisnode = top->first;
            auto *nbrs = &(top->second);
            // FIXME: to support token swap, nbrs should be determined by a customized function
            if (!nbrs->empty()) {
                int nextnode = nbrs->front();
                nbrs->pop();
                if (nextnode == startnode) {
                    allCycles.push_back(path);
                    for (auto node : path) {
                        closed.insert(node);
                    }
                } else if (blocked.find(nextnode) == blocked.end()) {
                    path.push_back(nextnode);
                    queue<int> nextnodeNbrs;
                    for (auto nbr : subgraph.get_neighbors(nextnode)) {
                        nextnodeNbrs.push(nbr);
                    }
                    st.push(make_pair(nextnode, nextnodeNbrs));
                    closed.erase(nextnode);
                    blocked.insert(nextnode);
                    continue;
                }
            }
            if (nbrs->empty()) {
                if (closed.find(thisnode) != closed.end()) {
                    unblock_(thisnode, blocked, B);
                } else {
                    for (auto nbr : subgraph.get_neighbors(thisnode)) {
                        if (B[nbr].find(thisnode) != B[nbr].end()) {
                            B[nbr].insert(thisnode);
                        }
                    }
                }
                st.pop();
                path.pop_back();
            }
        }
        Graph H = subgraph.generate_subgraph(scc);  // make smaller to avoid work in SCC routine
        auto H_sccs = H.strongly_connected_components();
        for (auto scc : H_sccs) {
            if (scc.size() > 1) {
                sccs.push_back(scc);
            }
        }
    }
    

    return allCycles;
}

void Graph::strongly_connected_components_util_(vector<unordered_set<int>> &sccs, int u,
                                                stack<int> &st,
                                                vector<int> &index, vector<int> &low,
                                                vector<bool> &onStack, vector<int> &time) {
    index[u] = low[u] = time[0];
    time[0] += 1;
    st.push(u);
    onStack[u] = true;

    for (auto nbr : get_neighbors(u)) {
        if (index[nbr] == -1) {
            strongly_connected_components_util_(sccs, nbr, st, index, low, onStack, time);
            low[u] = min(low[u], low[nbr]);
        } else if (onStack[nbr]) {
            low[u] = min(low[u], index[nbr]);
        }
    }

    if (low[u] == index[u]) {
        unordered_set<int> scc;
        while (1) {
            int v = st.top();
            st.pop();
            onStack[v] = false;
            scc.insert(v);
            if (u == v) break;
        }
        sccs.push_back(scc);
    }
}

// Tarjan's Strongly Connected Components Algorithm
vector<unordered_set<int>> Graph::strongly_connected_components() {
    vector<unordered_set<int>> sccs;

    vector<int> index(numNodes_, -1);
    vector<int> low(numNodes_, -1);
    vector<bool> onStack(numNodes_, false);
    vector<int> time = {0};
    stack<int> st;
    for (int i = 0; i < numNodes_; ++i) {
        if (index[i] == -1) {
            strongly_connected_components_util_(sccs, i, st, index, low, onStack, time);
        }
    }

    return sccs;
}

// This subgraph copies edges while not changing the index, so the total number of vertices
// is the same as the original graph.
Graph Graph::generate_subgraph(unordered_set<int> nodes) {
    Graph subgraph(numNodes_, directed_);
    for (auto edge : get_edges()) {
        int u = edge[0], v = edge[1];
        if (nodes.find(u) != nodes.end() && nodes.find(v) != nodes.end()) {
            subgraph.add_edge(u, v);
        }
    }
    return subgraph;
}

}  // namespace: qaoagraph