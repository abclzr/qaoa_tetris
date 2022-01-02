#include "Graph.hpp"
#include <iostream>
#include <queue>

using namespace std;

bool Graph::add_node(int u) {
    if (u < 0) return false;
    if (u >= numNodes_) {
        numNodes_ = u + 1;
        adjList_.resize(numNodes_);
        in_degree_.resize(numNodes_, 0);
        out_degree_.resize(numNodes_, 0);
    }
    return true;
}


bool Graph::add_edge(int u, int v) {
    if (u < 0 || v < 0) return false;
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
        numEdges_ += 1;
        out_degree_[v] += 1;
        in_degree_[u] += 1;        
    }
    return true;
}


void Graph::remove_nodes(set<int> nodes) {
    // FIXME: we are actually removing edges related to the node but 
    // not remove nodes.
    for (int i = 0; i < numNodes_; i++) {
        if (nodes.find(i) != nodes.end()) {
            numEdges_ -= adjList_[i].size();
            out_degree_[i] -= adjList_[i].size();
            for (auto node : adjList_[i]) {
                in_degree_[node] -= 1;
            }
            adjList_[i] = set<int>();
        } else {
            set<int> i_nbrs = adjList_[i];
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
    if (!directed_) return out_degree(u);
    else return out_degree(u) + in_degree(u);
}


bool Graph::load_from_file(ifstream &graphFile, bool directed) {
    directed_ = directed;
    int numEdges;
    graphFile >> numNodes_ >> numEdges;
    adjList_.resize(numNodes_);
    in_degree_.resize(numNodes_, 0);
    out_degree_.resize(numNodes_, 0);
    
    // FIXME: check if the edge number is correct.
    for (int i = 0; i < numEdges; i++) {
        int n1, n2;
        graphFile >> n1 >> n2;
        if (n1 >= numNodes_ || n2 >= numNodes_) {
            return false;
        }
        if (!add_edge(n1, n2)) {
            return false;
        }
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
        for (auto vi = adjList_[u].begin(); vi != adjList_[u].end(); ++vi) {
            if (visited[*vi] == 0) {
                visited[*vi] = 1;
                q.emplace(*vi);
                g.add_edge(u, *vi);
            } else if (visited[*vi] == 1) {
                g.add_edge(u, *vi);
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


void Graph::topo_sort_util(int u, vector<bool> &visited, vector<int>& topo_order) {
    // Mark the current node as visited.
    visited[u] = true;
 
    // Recur for all the vertices
    // adjacent to this vertex
    for (auto vi = adjList_[u].begin(); vi != adjList_[u].end(); ++vi)
        if (!visited[*vi])
            topo_sort_util(*vi, visited, topo_order);
 
    // Push current vertex to stack
    // which stores result
    topo_order.insert(topo_order.begin(), u);   
}


vector<int> Graph::get_topo_order() {
    vector<int> topo_order;
    vector<bool> visited(numNodes_, false);

    for (int i = 0; i < numNodes_; i++) {
        if (!visited[i]) {
            topo_sort_util(i, visited, topo_order);
        }
    }

    return topo_order;
}


vector<int> Graph::get_reversed_topo_order() {
    vector<int> topo_order = get_topo_order();
    return vector<int>(topo_order.rbegin(), topo_order.rend());
}


set<int> Graph::get_candidate_set(int u, Graph &g) {
    int u_degree = degree(u);
    set<int> candidate_set;
    for (int i = 0; i < g.num_nodes(); i++) {
        if (u_degree <= g.degree(i)) {
            candidate_set.insert(i);
        }
    }
    return candidate_set;
}