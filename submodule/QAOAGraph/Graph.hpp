#ifndef GRAPH_H
#define GRAPH_H
#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <stack>
#include <unordered_set>
#include <unordered_map>

using namespace std;

namespace qaoagraph {
    
class Graph {
private:
    int numNodes_;
    int numEdges_;
    bool directed_;
    vector<int> in_degree_;
    vector<int> out_degree_;
    vector<unordered_set<int>> adjList_;

    bool topo_order_need_update_ = true;
    vector<int> topo_order_;
    vector<int> rev_topo_order_;

    void update_topo_order();
    void topo_sort_util(int v, vector<bool> &visited, vector<int> &topo_order);

    void strongly_connected_components_util_(vector<unordered_set<int>> &scc, int u,
                                             stack<int> &st,
                                             vector<int> &index, vector<int> &low,
                                             vector<bool> &onStack, vector<int> &time);

public:
    Graph() : numNodes_(0), numEdges_(0), directed_(false) {}
    Graph(bool directed) : numNodes_(0), numEdges_(0), directed_(directed) {}
    Graph(int numNodes, bool directed = false) : numNodes_(numNodes),
                                                 numEdges_(0),
                                                 directed_(directed) {
        adjList_.resize(numNodes);
        in_degree_.resize(numNodes, 0);
        out_degree_.resize(numNodes, 0);
    }

    ~Graph(){};

    int num_nodes() { return numNodes_; }
    int num_nonempty_nodes();
    int num_edges() { return numEdges_; }

    bool add_node(int u);
    bool add_edge(int u, int v);

    void remove_nodes(unordered_set<int> nodes);

    vector<vector<int>> get_edges();
    const unordered_set<int> &get_neighbors(int u) const { return adjList_[u]; }

    bool has_edge(int u, int v) {
        return adjList_[u].find(v) != adjList_[u].end();
    }

    void print_adjList() {
        for (int i = 0; i < numNodes_; i++) {
            cout << "Node " << i << ": ";
            for (auto j : adjList_[i]) {
                cout << j << ", ";
            }
            cout << endl;
        }
    }

    int in_degree(int u);
    int out_degree(int u);
    int degree(int u);

    bool load_from_file(ifstream &graphFile, bool directed = false);

    Graph generate_dag(int u);
    Graph generate_reversed_graph();

    vector<int> &get_topo_order();
    vector<int> &get_reversed_topo_order();

    vector<unordered_set<int>> strongly_connected_components();

    vector<vector<int>> simple_cycles();

    unordered_set<int> get_candidate_set(int u, Graph &g);

    Graph generate_subgraph(unordered_set<int> nodes);
};

}  // namespace: qaoagraph

#endif