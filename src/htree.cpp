#include "htree.h"

HTree::HTree(const std::array<uint32_t, CodeNum>& freqs) { root_ = build(freqs); }

struct HNodeCmp {
    bool operator()(const HNode* lhs, const HNode* rhs) { return (lhs->freq > rhs->freq); }
};

std::unique_ptr<HNode> HTree::build(const std::array<uint32_t, CodeNum>& freqs) {
    MinHeap<HNode*, HNodeCmp> heap;

    for (uint32_t i = 0; i < CodeNum; ++i) {
        if (freqs[i]) heap.insert(new HNode(true, static_cast<char>(i), freqs[i]));
    }

    while (heap.size() > 1) {
        HNode* cl = heap.extract();
        HNode* cr = heap.extract();

        heap.insert(new HNode(false, cl->freq + cr->freq, cl, cr));
    }

    return std::unique_ptr<HNode>(heap.extract());
}