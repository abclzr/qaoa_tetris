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
#include <set>
#include <unordered_map>
#include <climits>
#include <utility>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include "Graph.hpp"
#include "BiMap.hpp"

using namespace qaoagraph;
using namespace bimap;

namespace subiso {

class GraphMatch
{
private:
	bool hasSubgraph_ = true;
	Graph queryG_;
	int rootIndex_;
	Graph queryDAG_;
	Graph revQueryDAG_;
	Graph dataG_;
	Graph csG_;
	vector<unordered_map<int, int>> uv2id_;
	unordered_map<int, pair<int, int>> id2uv_;
	unordered_map<int, int>  weightArray_;


	bool refine_CS(Graph &initCS, 
					vector<unordered_map<int, int>> &uv2id,
					unordered_map<int, pair<int, int>> &id2uv,
					Graph &queryDAG);
	void build_CS(bool enable_refine = false);

	void update_CS(bool enable_refine = false);

	int get_root_node();

public:

	int bt_count = 0;

	bool early_check();

	GraphMatch(Graph queryG, Graph dataG, int rootIndex = -1) : 
		queryG_(queryG), dataG_(dataG) {
		if (!early_check()) {
			hasSubgraph_ = false;
			return;
		}

		// Step 1: Build the dag graph for the query graph.
		// Here we use a sequence of vertex/index instead of actually generating a graph.
		rootIndex_ = rootIndex == -1 ? get_root_node() : rootIndex;
		queryDAG_ = queryG_.generate_dag(rootIndex_);
		revQueryDAG_ = queryDAG_.generate_reversed_graph();
		dataG_.generate_edge_checker();
		srand((unsigned)time(NULL));
    	build_CS();
	}
	GraphMatch() {};
	~GraphMatch() {};

	int get_root_index() const { return rootIndex_; }

	void set_root_index(int rootIndex) { rootIndex_ = rootIndex; }

	Graph &get_query_G() {return queryG_;}
	Graph &get_query_dag() {return queryDAG_;}
	Graph &get_rev_query_dag() {return revQueryDAG_;}
	Graph &get_data_G() {return dataG_;}

	bool update_data_G(Graph dataG);

	bool backtrack(BiMap &M, 
					vector<BiMap> &allM, 
					unordered_set<int> expendable_u,
					unordered_map<int,int> indegrees,
					int count=INT_MAX);

	// XXX: Now a vector of int pairs is used to represent the matching from query to data. 
	// It suppose to return an iterator(?) for practical use.
	vector<BiMap> subgraph_isomorphsim(int count=INT_MAX);


	// XXX: These functions are supposed to be private (also for these args that are private variables)
	// but for testing issue, they are public now check the following link to improve.
	// https://stackoverflow.com/questions/47354280/what-is-the-best-way-of-testing-private-methods-with-googletest
	


	pair<int, vector<int>> get_next_node(BiMap &M, 
                                Graph &queryDAG,
                                Graph &revQueryDAG,
								unordered_set<int> &expendable_u,
                                Graph &CS,
                                vector<unordered_map<int, int>> &uv2id, 
                                unordered_map<int, pair<int, int>> &id2uv,
                                unordered_map<int,int> &weightArray);

	void update_init_CS();

	void build_init_CS(Graph &CS, 
                        vector<unordered_map<int, int>> &uv2id, 
                        unordered_map<int, pair<int, int>> &id2uv);

	bool refine_CS_wrapper(Graph &CS, 
							vector<unordered_map<int, int>> &uv2id,
							unordered_map<int, pair<int, int>> &id2uv,
							Graph &queryDAG,
							int reversed);

	void build_weight_array(unordered_map<int, int> &weightedArray,
							Graph &queryDAG, 
							Graph &CS,
							vector<unordered_map<int, int>> &uv2id,
							unordered_map<int, pair<int, int>> &id2uv);

};

} // namespace: subiso

#endif

