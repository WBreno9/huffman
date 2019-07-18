#ifndef HTREE_H
#define HTREE_H

#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <vector>

#include "heap.h"

struct HNode {
    bool isLeaf;
    char byte;
    uint32_t freq;

    std::unique_ptr<HNode> left;
    std::unique_ptr<HNode> right;

    HNode(bool isLeaf, char byte, uint32_t freq) : isLeaf(isLeaf), byte(byte), freq(freq) {}
    HNode(bool isLeaf, uint32_t freq, HNode* left, HNode* right)
        : isLeaf(isLeaf), byte(0), freq(freq), left(left), right(right) {}
};

struct HNodeCmp {
    bool operator()(const HNode* lhs, const HNode* rhs) { return (lhs->freq > rhs->freq); }
};

constexpr uint32_t CodeNum = 256;

class HTree {
   public:
    HTree(const std::array<uint32_t, CodeNum>& freqs);
    std::unique_ptr<HNode> build(const std::array<uint32_t, CodeNum>& freqs);

    std::unique_ptr<HNode> root_;
};

#endif // !HTREE_H
