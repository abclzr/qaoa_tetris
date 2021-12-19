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
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>

using namespace boost;
using namespace std;
	
/**
 * @brief Current graph implementation is based on boost graph library.
 * In the future, it's possible to switch to other library(e.g. Lemon).
 * 
 */
class Graph  
{
	private:

		
	public:
		typedef adjacency_list<vecS, vecS, undirectedS> Graph_bst;
		typedef adjacency_list<vecS, vecS, directedS> DGraph_bst;

		// boost use bfs_visitor to define operation at different stages of a bfs.
		// Check https://www.boost.org/doc/libs/1_39_0/libs/graph/doc/BFSVisitor.html for 
		// the definiation of tree_edge and gray_target.
		class MyVisitor : public boost::default_bfs_visitor {
			private:
				DGraph_bst &dgraph_;
			public:

				MyVisitor(int n, DGraph_bst &dgraph) : dgraph_(dgraph) {
					dgraph_ = DGraph_bst(n);
				}

				~MyVisitor() {}
				
				void tree_edge(const Graph_bst::edge_descriptor &e, const Graph_bst &g) {
					add_edge(e.m_source, e.m_target, dgraph_);
				}

				void gray_target(const Graph_bst::edge_descriptor &e, const Graph_bst &g) {
					add_edge(e.m_source, e.m_target, dgraph_);
				}
		};

		Graph_bst graph_;
		DGraph_bst dgraph_;

		Graph() : graph_(Graph_bst(0)) {}
		Graph(Graph_bst graph);
		~Graph();

		/**
		 * @brief 
		 * 
		 * @param graphFile The graph file in the format of edgelist. 
		 * First line of the file is num_vertices and num_edges.
		 * @return true If the generated graph is valid (vertex and edge number matches).
		 * @return false Otherwise.
		 */
		bool load_from_file(ifstream &graphFile);

		// XXX: DAG Graph could be a seperate class that inheritance this graph class.
		DGraph_bst generate_dag(int rootIndex);

};
#endif