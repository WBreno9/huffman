#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <array>
#include <queue>
#include <map>
#include <iterator>
#include <bitset>

#include "heap.h"

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
        std::priority_queue<HNode*, std::vector<HNode*>, HNodeCmp> tree;

        std::vector<HNode*> nodes;
        for (uint32_t i = 0; i < CodeNum; ++i) {
            if (freqs[i]) {
                nodes.push_back(new HNode(true, static_cast<char>(i), freqs[i]));
            }
        }

		//MinHeap<HNode*, HNodeCmp> heap(nodes);		
		MinHeap<HNode*, HNodeCmp> heap;		

        for (uint32_t i = 0; i < CodeNum; ++i) {
            if (freqs[i]) {
				heap.insert(new HNode(true, static_cast<char>(i), freqs[i]));
			}
        }

        // while(!heap.is_empty()) {
        //     HNode* n = heap.extract();
        //     std::cout << static_cast<uint32_t>(n->byte) << "\t" << n->freq << " " << heap.size() << "\n";
        // }

        for (uint32_t i = 0; i < CodeNum; ++i) {
            if (freqs[i]) {
				// heap.insert(new HNode(true, static_cast<char>(i), freqs[i]));
                tree.push(new HNode(true, static_cast<char>(i), freqs[i]));
			}
        }

        //while (tree.size() > 1) {
        while (heap.size() > 1) {
            HNode* cl = nullptr;
            HNode* cr = nullptr;

            cl = heap.extract();
            cr = heap.extract();

            //std::cout << "Left: " << static_cast<uint32_t>(cl->byte) << " Right: " << static_cast<uint32_t>(cr->byte) << " | ";

            // cl = tree.top();
            // tree.pop();

            // cr = tree.top();
            // tree.pop();

            //std::cout << "Left: " << static_cast<uint32_t>(cl->byte) << " Right: " << static_cast<uint32_t>(cr->byte) << "\n";


            auto p = new HNode(false, cl->freq + cr->freq, cl, cr);
            // tree.push(p);
            heap.insert(p);
        }

        // return std::unique_ptr<HNode>(tree.top());
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
