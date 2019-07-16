#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <array>
#include <queue>
#include <map>
#include <iterator>
#include <bitset>

struct HNode {
	uint32_t id;
    bool isLeaf;
    char byte;
    uint32_t freq;

    std::unique_ptr<HNode> left;
    std::unique_ptr<HNode> right;

    HNode(bool isLeaf, char byte, uint32_t freq)
        : isLeaf(isLeaf), byte(byte), freq(freq) {}
    HNode(bool isLeaf, uint32_t freq, HNode* left, HNode* right)
        : isLeaf(isLeaf), byte(0), freq(freq), left(left), right(right) {}
};

class MinHeap {
   public:
    MinHeap(const std::vector<HNode>& nodes_) : nodes(nodes_) {
        pos.resize(nodes.size());
        uint32_t k = 0;
        for (auto& p : pos) p = k++;

        size = nodes.size();

        uint32_t j = (size / 2.0f) - 1;
        for (uint32_t i = j + 1; i != 0; --i) heapify(i - 1);
    }

    HNode extract() {
        HNode extracted = nodes[0];

        pos[nodes[size - 1].id] = 0;
        pos[extracted.id] = size - 1;

        std::swap(nodes[0], nodes[size - 1]);
        --size;

        heapify(0);
        return extracted;
    }

    void heapify(uint32_t index) {
        uint32_t t = index;

        while (true) {
            uint32_t l = left_child(t);
            uint32_t r = right_child(t);

            if ((l < size) && (nodes[t].freq > nodes[l].freq)) t = l;
            if ((r < size) && (nodes[t].freq > nodes[r].freq)) t = r;

            if (t != index) {
                pos[nodes[t].id] = index;
                pos[nodes[index].id] = t;

                std::swap(nodes[index], nodes[t]);
                index = t;
            } else {
                break;
            }
        }
    }

    bool decrement(uint32_t id, uint32_t freq) {
        uint32_t index = pos[id];

        if ((index >= size) || (nodes[index].freq < freq)) return false;

        nodes[index].freq = freq;

        while (index && (nodes[index].freq < nodes[parent(index)].freq)) {
            pos[nodes[index].id] = parent(index);
            pos[nodes[parent(index)].id] = index;

            uint32_t p_id = nodes[parent(index)].id;

            std::swap(nodes[index], nodes[parent(index)]);

            index = pos[p_id];
        }
    
        return true;
    }
    
    void insert(HNode& node) 
		node.id = nodes.size() - 1;
		pos.push_back(node.id);
        nodes.push_back(node);

		uint32_t index = nodes.size() - 1;
        while (index && (nodes[index].freq < nodes[parent(index)].freq)) {
            pos[nodes[index].id] = parent(index);
            pos[nodes[parent(index)].id] = index;

            uint32_t p_id = nodes[parent(index)].id;

            std::swap(nodes[index], nodes[parent(index)]);
            index = pos[p_id];
        }
    }

    std::vector<HNode> get_nodes() const {
        return std::vector<HNode>(nodes.begin(), (nodes.begin() + size));
    }
    bool is_empty() const { return size == 0; }

   private:
    std::vector<uint32_t> pos;
    std::vector<HNode> nodes;

    uint32_t size;

    uint32_t parent(uint32_t index) const { return index / 2.0f; }
    uint32_t left_child(uint32_t index) const { return 2 * index + 1; }
    uint32_t right_child(uint32_t index) const { return 2 * index + 2; }
};

struct HNodeCmp {
    bool operator()(const HNode* lhs, const HNode* rhs) {
        return (lhs->freq > rhs->freq);
    }
};

constexpr uint32_t CodeNum = 256;

class HTree {
   public:
    HTree(const std::array<uint32_t, CodeNum>& freqs) { root_ = build(freqs); }

    std::unique_ptr<HNode> build(const std::array<uint32_t, CodeNum>& freqs) {
        // std::priority_queue<HNode*, std::vector<HNode*>, HNodeCmp> tree;
		HeapMin heap;		

        for (uint32_t i = 0; i < CodeNum; ++i) {
            if (freqs[i]) {
				heap.insert(new HNode(true, static_cast<char>(i), freqs[i]));
                // tree.push(new HNode(true, static_cast<char>(i), freqs[i]));
			}
        }

        while (tree.size() > 1) {
            HNode* cl = tree.top();
            tree.pop();

            HNode* cr = tree.top();
            tree.pop();

            auto p = new HNode(false, cl->freq + cr->freq, cl, cr);
            tree.push(p);
        }

        return std::unique_ptr<HNode>(tree.top());
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

void generateCodes(const std::unique_ptr<HNode>& node, const HCode& prefix,
                   HCodeMap& outCodes) {
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

std::vector<uint8_t> decode(const HTree& tree,
                            const std::vector<uint8_t>& data) {
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
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(input_file)),
                              (std::istreambuf_iterator<char>()));
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
