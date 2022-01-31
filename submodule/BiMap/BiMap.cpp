#include "BiMap.hpp"  

#include <iostream>

using namespace std;

namespace bimap {

void BiMap::update(int key, int value) { 
    mapping_[key] = offset_ + value; 
    mapping_[offset_ + value] = key;
}

void BiMap::eraseByKey(int key) {
    mapping_.erase(mapping_[key]);
    mapping_.erase(key);
}

void BiMap::eraseByValue(int value) {
    eraseByKey(mapping_[offset_ + value]);
}

int BiMap::getKeyByValue(int value) {
    return mapping_[offset_ + value];
}

int BiMap::getValueByKey(int key) {
    return mapping_[key] - offset_;
}


void BiMap::swapValueByKey(int key1, int key2) {
    int value1 = getValueByKey(key1);
    int value2 = getValueByKey(key2);
    update(key1, value2);
    update(key2, value1);
}


bool BiMap::hasKey(int key) {
    return mapping_.find(key) != mapping_.end();
}

bool BiMap::hasValue(int value) {
    return mapping_.find(offset_ + value) != mapping_.end();
}

void BiMap::print() {
    for (auto it : mapping_) {
        if (it.first >= offset_) continue;
        printf("key: %d val: %d | ", it.first, it.second  - offset_ );
    }
    cout << endl;
}


bool BiMap::load_from_file(ifstream &mappingFile) {
    mappingFile >> offset_;

    int key = 0;
    int value;
    while (mappingFile >> value) {
        // NOTE: Currently we only consider 1-1 mapping.
        if (key >= offset_ || value >= offset_ || value < 0) {
            return false;
        }
        update(key, value);
        key += 1;
    }

    return true;
}

}  // namespace: bimap