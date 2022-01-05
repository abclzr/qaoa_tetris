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
#include <climits>

#include "Graph.hpp"
#include "Mapping.hpp"

class GraphMatch
{
private:
	Graph queryG_;
	Graph queryDAG_;
	Graph revQueryDAG_;
	Graph dataG_;
	Graph csG_;
	unordered_map<int, unordered_map<int, int>> uv2id_;
	unordered_map<int, pair<int, int>> id2uv_;
	unordered_map<int, int>  weightArray_;


	bool refine_CS(Graph &initCS, 
					unordered_map<int, unordered_map<int, int>> &uv2id,
					unordered_map<int, pair<int, int>> &id2uv,
					Graph &queryDAG);
	void build_CS();

	int get_root_node();

public:
	GraphMatch(Graph queryG, Graph dataG) : 
		queryG_(queryG), dataG_(dataG) {
		// Step 1: Build the dag graph for the query graph.
		// Here we use a sequence of vertex/index instead of actually generating a graph.
		int rootIndex = get_root_node();
		queryDAG_ = queryG_.generate_dag(rootIndex);
		revQueryDAG_ = queryDAG_.generate_reversed_graph();
	}
	GraphMatch() {};
	~GraphMatch() {};

	Graph &get_query_G() {return queryG_;}
	Graph &get_query_dag() {return queryDAG_;}
	Graph &get_rev_query_dag() {return revQueryDAG_;}
	Graph &get_data_G() {return dataG_;}

	void backtrack(Mapping &M, 
					vector<Mapping> &allM, 
					int count=INT_MAX);

	// XXX: Now a vector of int pairs is used to represent the matching from query to data. 
	// It suppose to return an iterator(?) for practical use.
	vector<Mapping> subgraph_isomorphsim(int count=INT_MAX);


	// XXX: These functions are supposed to be private (also for these args that are private varaibles)
	// but for testing issue, they are public now check the following link to improve.
	// https://stackoverflow.com/questions/47354280/what-is-the-best-way-of-testing-private-methods-with-googletest
	

	int get_next_node(Mapping &M,
						Graph &queryDAG, 
						Graph &CS,
                        unordered_map<int, unordered_map<int, int>> &uv2id, 
                        unordered_map<int, pair<int, int>> &id2uv);

	void build_init_CS(Graph &CS, 
                        unordered_map<int, unordered_map<int, int>> &uv2id, 
                        unordered_map<int, pair<int, int>> &id2uv);

	bool refine_CS_wrapper(Graph &CS, 
							unordered_map<int, unordered_map<int, int>> &uv2id,
							unordered_map<int, pair<int, int>> &id2uv,
							Graph &queryDAG,
							int reversed);

	void build_weight_array(unordered_map<int, int> &weightedArray,
							Graph &queryDAG, 
							Graph &CS,
							unordered_map<int, unordered_map<int, int>> &uv2id,
							unordered_map<int, pair<int, int>> &id2uv);

};
#endif