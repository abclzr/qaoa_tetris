#ifndef MAPPING_H
#define MAPPING_H
#pragma once

#include  <unordered_map>

using namespace std;

// FIXME: add tests for this class.
// XXX: add description
// simple idea: we have limited number of nodes in query and data graph. So we 
// don't need two dicts for both direction. we add an offset to one graph's index
// so we only need one dict now.
// XXX: If use template so uv2id/id2uv can also use this.
class Mapping  
{
	private:
		int offset_;
		unordered_map<int, int> mapping_;

	public:

		Mapping(int offset) : offset_(offset) {};
		~Mapping() {};

		void update(int queryIdx, int dataIdx);

		void eraseByQueryIdx(int queryIdx);

		void eraseByDataIdx(int dataIdx);

		int getQueryIdx(int dataIdx);

		int getDataIdx(int queryIdx);

		int size() { return mapping_.size() / 2; }

		bool findQueryIdx(int queryIdx);

		bool findDataIdx(int dataidx);

		void print();
};
#endif