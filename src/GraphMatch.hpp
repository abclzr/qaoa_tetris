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

#include "Graph.hpp"

class GraphMatch
{
private:
	Graph queryG_;
	Graph dataG_;
	Graph::DGraph_bst queryDAG_;

	Graph::DGraph_bst build_candidate_space();

	int getRootNode();


	
public:
	GraphMatch(Graph queryG, Graph dataG) : 
		queryG_(queryG), dataG_(dataG) {
		// Step 1: Build the dag graph for the query graph.
		int rootIndex = getRootNode();
		queryDAG_ = queryG_.generate_dag(rootIndex);
	}

	~GraphMatch() {}

	// XXX: Now a vector of int pairs is used to represent the matching from query to data. 
	// It suppose to return an iterator(?) for practical use.
	vector<vector<pair<int, int>>> subgraph_isomorphsim();
};
#endif