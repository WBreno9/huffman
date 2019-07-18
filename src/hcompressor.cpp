#include "hcompressor.h"

HCompressor::HCompressor() {}
HCompressor::~HCompressor() {}

std::vector<uint8_t> HCompressor::compressData(const std::vector<uint8_t>& data) {
    std::array<uint32_t, CodeNum> freqs = {0};
    for (auto c : data) ++freqs[c];

    HTree tree(freqs);

    HCodeMap codes;
    generateCodes(tree.root_, HCode(), codes);

    std::vector<uint8_t> encoded = encode(codes, data);

    std::vector<uint8_t> encoded_data(encoded.size() + CodeNum * sizeof(uint32_t));

    std::memcpy(encoded_data.data(), freqs.data(), CodeNum * sizeof(uint32_t));
    std::memcpy(encoded_data.data() + CodeNum * sizeof(uint32_t), encoded.data(), encoded.size());

    return encoded_data;
}

std::vector<uint8_t> HCompressor::uncompressData(const std::vector<uint8_t>& data) {
    std::array<uint32_t, CodeNum> freqs = {0};
    std::vector<uint8_t> encoded(data.size() - CodeNum * sizeof(uint32_t));

    std::memcpy(freqs.data(), data.data(), CodeNum * sizeof(uint32_t));
    std::memcpy(encoded.data(), data.data() + CodeNum * sizeof(uint32_t), encoded.size());

    HTree tree(freqs);

    std::vector<uint8_t> decoded = decode(tree, encoded);

    return decoded;
}

std::vector<uint8_t> HCompressor::compressFile(const std::string& file_path) {
    std::ifstream input_file(file_path);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(input_file)), (std::istreambuf_iterator<char>()));
    input_file.close();

    return compressData(data);
}

std::vector<uint8_t> HCompressor::uncompressFile(const std::string& file_path) {
    std::ifstream input_file(file_path, std::ios::binary);

    input_file.seekg(0, input_file.end);
    uint32_t size = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    std::vector<uint8_t> data;
    data.resize(size);
    input_file.read(reinterpret_cast<char*>(data.data()), data.size());

    return uncompressData(data);
}

void HCompressor::compressFileWrite(const std::string& file_path, const std::string& write_file_path) {
    auto data = compress_file(file_path);
    std::ofstream fs(write_file_path);
    fs.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void HCompressor::uncompressFileWrite(const std::string& file_path, const std::string& write_file_path) {
    auto data = uncompress_file(file_path);
    std::ofstream fs(write_file_path);
    fs.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void HCompressor::generateCodes(const std::unique_ptr<HNode>& node, const HCode& prefix, HCodeMap& outCodes) {
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

std::vector<uint8_t> HCompressor::encode(HCodeMap& codes, const std::vector<uint8_t>& data) {
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
    if (byte) encoded.push_back(byte);

    return encoded;
}

std::vector<uint8_t>HCompressor::decode(const HTree& tree, const std::vector<uint8_t>& data) {
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
    if (n->isLeaf) decoded.push_back(n->byte);

    return decoded;
}
