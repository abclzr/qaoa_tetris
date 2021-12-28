/**
 * @file GraphMatch.hpp
 * @author Yanhao Chen (chenyh64@gmail.com)
 * @brief The graph matcher class. Follow the name used in python networkx package. 
 * It implements the DAF subgraph isomorphism. 
 * @version 0.1
 * @date 2021-12-18
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef GRAPHMATCH_H
#define GRAPHMATCH_H
#pragma once

#include <vector>
#include <unordered_map>

#include "Graph.hpp"

class GraphMatch
{
private:
	Graph queryG_;
	Graph queryDAG_;
	Graph dataG_;
	Graph csG_;
	Graph weightArray_;


	bool refine_CS(Graph &initCS, 
					unordered_map<int, unordered_map<int, int>> &uv2id,
					unordered_map<int, pair<int, int>> &id2uv,
					Graph &queryDAG);
	void build_CS();

	int getRootNode();

public:
	GraphMatch(Graph queryG, Graph dataG) : 
		queryG_(queryG), dataG_(dataG) {
		// Step 1: Build the dag graph for the query graph.
		// Here we use a sequence of vertex/index instead of actually generating a graph.
		int rootIndex = getRootNode();
		queryDAG_ = queryG_.generate_dag(rootIndex);
	}
	GraphMatch() {};
	~GraphMatch() {};

	Graph &get_query_G() {return queryG_;}
	Graph &get_query_dag() {return queryDAG_;}
	Graph &get_data_G() {return dataG_;}

	// XXX: Now a vector of int pairs is used to represent the matching from query to data. 
	// It suppose to return an iterator(?) for practical use.
	vector<vector<pair<int, int>>> subgraph_isomorphsim();

	void build_init_CS(Graph &initCS, 
                        unordered_map<int, unordered_map<int, int>> &uv2id, 
                        unordered_map<int, pair<int, int>> &id2uv);

	bool refine_CS_wrapper(Graph &initCS, 
							unordered_map<int, unordered_map<int, int>> &uv2id,
							unordered_map<int, pair<int, int>> &id2uv,
							Graph &queryDAG,
							int reversed);

};
#endif