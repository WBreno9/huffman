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

        for (uint32_t i = 0; i < CodeNum; ++i) {
            if (freqs[i])
                tree.push(new HNode(true, static_cast<char>(i), freqs[i]));
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
    inline void insert_0() {
        nBits_++;
    }

    std::vector<bool> gv() {
        std::vector<bool> o;
        o.reserve(nBits_);
        for (int64_t i = 0; i < nBits_; ++i)
            o.push_back(((codeb_ & (1 << (31 - i))) ? true : false));
        return o;
    }
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

std::vector<bool> encode(HCodeMap& codes, const std::vector<uint8_t>& data) {
    std::vector<bool> encoded;

    for (auto byte : data) {
        std::vector<bool> gv = codes[byte].gv();
        encoded.insert(encoded.end(), gv.begin(), gv.end());
    }

    std::vector<uint8_t> out;

    for (uint32_t i = 0; i < data.size(); ++i) {
        uint8_t dbyte = data[i];
        HCode code = codes[dbyte];

        for (uint32_t j = 0; j < code.nBits_; ++j) {

        }
    }

    return encoded;
}

std::vector<uint8_t> decode(const HTree& tree, const std::vector<bool>& data) {
    std::vector<uint8_t> decoded;
    HNode* n = tree.root_.get();

    auto it = data.begin();
    while (it != data.end()) {
        if (n->isLeaf) {
            decoded.push_back(n->byte);
            n = tree.root_.get();
        } else {
            if (*it)
                n = n->left.get();
            else
                n = n->right.get();
            ++it;
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
    // for (uint32_t i = 0; i < CodeNum; ++i) std::cout << i << ": " << freqs[i]
    // << "\n";

    HTree tree(freqs);

    HCodeMap codes;
    generateCodes(tree.root_, HCode(), codes);

    for (auto& c : codes) {
        c.second.gv();
    }

    //  for (HCodeMap::const_iterator it = codes.begin(); it != codes.end();
    //  ++it) {
    //      std::cout << static_cast<uint32_t>(it->first) << " ";
    //      std::copy(it->second.begin(), it->second.end(),
    //                std::ostream_iterator<bool>(std::cout));
    //      std::cout << std::endl;
    //  }

    // std::vector<char> data(str.begin(), str.end());
    std::vector<bool> encoded = encode(codes, data);
    // for (auto bit : encoded) std::cout << bit;
    // std::cout << std::endl;

    std::vector<uint8_t> decoded = decode(tree, encoded);
    // for (auto byte : decoded) std::cout << static_cast<char>(byte);
    // std::cout << std::endl;

    std::ofstream output_file("out.huff");
    for (auto c : decoded) output_file << c;
    output_file.close();

    std::exit(EXIT_SUCCESS);
}