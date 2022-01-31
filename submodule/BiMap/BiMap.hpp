#ifndef BIMAP_H
#define BIMAP_H
#pragma once

#include <unordered_map>
#include <fstream>

using namespace std;

namespace bimap {

// This class serves as a convenient wrapper for a bi-direction mapping between two value.
// Currently only support the mapping between two integer.
// TODO: Extend this class to support templates.
class BiMap  
{
	private:
		int offset_;
		unordered_map<int, int> mapping_;

	public:

		BiMap() {};
		BiMap(int offset) : offset_(offset) {};
		~BiMap() {};

		void update(int key, int val);

		void eraseByKey(int key);

		void eraseByValue(int value);

		int getKeyByValue(int value);

		int getValueByKey(int key);

		void swapValueByKey(int key1, int key2);

		int size() { return mapping_.size() / 2; }

		bool hasKey(int key);

		bool hasValue(int value);

		void print();

		bool load_from_file(ifstream &mappingFile);
};
#endif

}  // namespace: bimap