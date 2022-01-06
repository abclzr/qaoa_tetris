#include "Mapping.hpp"  
#include <iostream>

using namespace std;
	
void Mapping::update(int queryIdx, int dataIdx) { 
    mapping_[queryIdx] = offset_ + dataIdx; 
    mapping_[offset_ + dataIdx] = queryIdx;
}

void Mapping::eraseByQueryIdx(int queryIdx) {
    mapping_.erase(mapping_[queryIdx]);
    mapping_.erase(queryIdx);
}

void Mapping::eraseByDataIdx(int dataIdx) {
    eraseByQueryIdx(mapping_[offset_ + dataIdx]);
}

int Mapping::getQueryIdx(int dataIdx) {
    return mapping_[offset_ + dataIdx];
}

int Mapping::getDataIdx(int queryIdx) {
    return mapping_[queryIdx] - offset_;
}

bool Mapping::findQueryIdx(int queryIdx) {
    return mapping_.find(queryIdx) != mapping_.end();
}

bool Mapping::findDataIdx(int dataIdx) {
    return mapping_.find(offset_ + dataIdx) != mapping_.end();
}

void Mapping::print() {
    for (auto it : mapping_) {
        if (it.first >= offset_) continue;
        cout << it.first << " " << it.second  - offset_ << " | ";
    }
    cout << endl;
}