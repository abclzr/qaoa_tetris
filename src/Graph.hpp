/**
 * @file Graph.hpp
 * @author Yanhao Chen (chenyh64@gmail.com)
 * @brief A class represents the data/query graph used in subgraph isomorphism
 * @version 0.1
 * @date 2021-12-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef GRAPH_H
#define GRAPH_H
#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <set>

using namespace std;

class Graph  
{
	private:
		int numNodes_;
		int numEdges_;
		bool directed_;
		vector<int> in_degree_;
		vector<int> out_degree_;
		vector<set<int>> adjList_;

	public:
		Graph() : numNodes_(0), numEdges_(0), directed_(false) {}

		Graph(int numNodes, bool directed=false) : 
			numNodes_(numNodes), 
			numEdges_(0), 
			directed_(directed) {
			adjList_.resize(numNodes);
			in_degree_.resize(numNodes, 0);
        	out_degree_.resize(numNodes, 0);
		}

		~Graph() {};

		int num_nodes() {return numNodes_;}
		int num_edges() {return numEdges_;}

		bool add_node(int u);
		bool add_edge(int u, int v);

		vector<vector<int>> get_edges();

		int in_degree(int u);
		int out_degree(int u);
		int degree(int u);

		bool load_from_file(ifstream &graphFile, bool directed=false);

		Graph generate_dag(int u);

		set<int> get_candidate_set(int u, Graph &g);

};
#endif