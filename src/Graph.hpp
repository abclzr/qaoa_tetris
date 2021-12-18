#ifndef GRAPH_H
#define GRAPH_H
#pragma once

#include <fstream>
#include <string>
#include <boost/graph/adjacency_list.hpp>

using namespace boost;
using namespace std;
	
class Graph  
{
	typedef adjacency_list<vecS, vecS, undirectedS> Graph_bst;

	private:
		

	public:

		Graph();
		~Graph();

		Graph_bst graph_;

		bool load_from_file(ifstream &graph_file);

};
#endif