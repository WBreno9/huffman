#include <array>
#include <bitset>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <vector>

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

struct HeapNode {
    uint32_t id;
    HNode* n;
};

class MinHeap {
   public:
    MinHeap() : size_(0) {}
    MinHeap(const std::vector<HeapNode>& nodes) : nodes_(nodes) {
        size_ = nodes_.size();

        uint32_t k = 0;
        for (auto& p : pos_)
            p = k++;

        uint32_t j = (size_ / 2.0f) - 1;
        for (uint32_t i = j + 1; i != 0; --i) heapify(i - 1);
    }

    HNode* extract() {
        HeapNode extracted = nodes_[0];

        pos_[nodes_[size_ - 1].id] = 0;
        pos_[extracted.id] = size_ - 1;

        std::swap(nodes_[0] ,nodes_[size_ - 1]);
        --size_;

        heapify(0);
        return extracted.n;
    }

    void heapify(uint32_t index) {
        if (size_ == 1) return;

        uint32_t t = index;

        while (true) {
            uint32_t l = left_child(t);
            uint32_t r = right_child(t);

            if ((r < size_) && (nodes_[t].n->freq > nodes_[r].n->freq)) t = r;
            if ((l < size_) && (nodes_[t].n->freq > nodes_[l].n->freq)) t = l;

            if (t != index) {
                pos_[nodes_[t].id] = index;
                pos_[nodes_[index].id] = t;
                std::swap(nodes_[t], nodes_[index]);
                index = t;
            } else {
                break;
            }
        }
    }

    void insert(HNode* v) {
        HeapNode n;
        n.n = v;
        n.id = size_;

        size_++;
        if (size_ < nodes_.size()) {
            nodes_[size_ - 1] = n;
            pos_[size_ - 1] = size_ - 1;
        } else {
            nodes_.push_back(n);
            pos_.push_back(size_ - 1);
        }

        uint32_t i = size_ - 1;

        while (i != 0 && (nodes_[parent(i)].n->freq > nodes_[i].n->freq)) {
            pos_[nodes_[i].id] = parent(i);
            pos_[nodes_[parent(i)].id] = i;

            uint32_t p_id = nodes_[parent(i)].id;

            std::swap(nodes_[i], nodes_[parent(i)]);

            i = pos_[p_id];
        }
    }

    std::vector<HeapNode> get_nodes() const { return std::vector<HeapNode>(nodes_.begin(), (nodes_.begin() + size_)); }
    bool is_empty() const { return size_ == 0; }
    uint32_t size() const { return size_; }

   private:
    std::vector<uint32_t> pos_;
    std::vector<HeapNode> nodes_;

    uint32_t size_;

    uint32_t parent(uint32_t index) const { return (index - 1) / 2.0f; }
    uint32_t left_child(uint32_t index) const { return 2 * index + 1; }
    uint32_t right_child(uint32_t index) const { return 2 * index + 2; }
};

constexpr uint32_t CodeNum = 256;

class HTree {
   public:
    HTree(const std::array<uint32_t, CodeNum>& freqs) { root_ = build(freqs); }

    std::unique_ptr<HNode> build(const std::array<uint32_t, CodeNum>& freqs) {
        MinHeap heap;

        for (uint32_t i = 0; i < CodeNum; ++i) {
            if (freqs[i]) {
                heap.insert(new HNode(true, static_cast<char>(i), freqs[i]));
            }
        }

        while (heap.size() > 1) {
            HNode* cl = heap.extract();
            HNode* cr = heap.extract();

            auto p = new HNode(false, cl->freq + cr->freq, cl, cr);
            heap.insert(p);
        }

        return std::unique_ptr<HNode>(heap.extract());
    }

    std::unique_ptr<HNode> root_;
};

struct HCode {
    uint32_t nBits_ = 0;
    uint32_t codeb_ = 0;

    inline void insert_1() {
        uint32_t i = 1 << (31 - nBits_);
        codeb_ |= i;
        nBits_++;
    }
    inline void insert_0() { nBits_++; }
};

using HCodeMap = std::map<char, HCode>;

void generateCodes(const std::unique_ptr<HNode>& node, const HCode& prefix, HCodeMap& outCodes) {
    if (node->isLeaf) {
        outCodes[node->byte] = prefix;
    } else {
        HCode leftPrefix = prefix;
        leftPrefix.insert_1();
        generateCodes(node->left, leftPrefix, outCodes);

        HCode rightPrefix = prefix;
        rightPrefix.insert_0();
        generateCodes(node->right, rightPrefix, outCodes);
    }
}

std::vector<uint8_t> encode(HCodeMap& codes, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> encoded;

    uint8_t byte = 0, bcounter = 0;
    for (uint32_t i = 0; i < data.size(); ++i) {
        HCode code = codes[data[i]];
        for (uint32_t j = 0; j < code.nBits_; ++j) {
            bool bl = (code.codeb_ & (1 << (31 - j)));
            byte |= bl << (7 - bcounter);

            if (bcounter == 7) {
                bcounter = 0;
                encoded.push_back(byte);
                byte = 0;
            } else {
                bcounter++;
            }
        }
    }
    encoded.push_back(byte);

    return encoded;
}

std::vector<uint8_t> decode(const HTree& tree, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> decoded;
    HNode* n = tree.root_.get();

    uint8_t bcounter = 0;
    auto it = data.begin();
    while (it != data.end()) {
        if (n->isLeaf) {
            decoded.push_back(n->byte);
            n = tree.root_.get();
        } else {
            if (*it & (1 << (7 - bcounter)))
                n = n->left.get();
            else
                n = n->right.get();

            if (bcounter == 7) {
                bcounter = 0;
                ++it;
            } else {
                bcounter++;
            }
        }
    }
    decoded.push_back(n->byte);

    return decoded;
}

int main(int argc, char** argv) {
    if (argc != 2) std::exit(EXIT_FAILURE);

    std::ifstream input_file(argv[1]);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(input_file)), (std::istreambuf_iterator<char>()));
    input_file.close();

    std::array<uint32_t, CodeNum> freqs = {0};

    for (auto c : data) ++freqs[c];

    HTree tree(freqs);

    HCodeMap codes;
    generateCodes(tree.root_, HCode(), codes);

    std::vector<uint8_t> encoded = encode(codes, data);
    std::vector<uint8_t> decoded = decode(tree, encoded);

    std::ofstream output_file("out.huff");
    for (auto c : decoded) output_file << c;
    output_file.close();

    std::exit(EXIT_SUCCESS);
}
