#ifndef HCOMPRESSOR_H
#define HCOMPRESSOR_H

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <map>
#include <cstring>

#include "htree.h"

class HCompressor {
   public:
    HCompressor();
    ~HCompressor();

    std::vector<uint8_t> compressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> uncompressData(const std::vector<uint8_t>& data);

    std::vector<uint8_t> compressFile(const std::string& file_path);
    std::vector<uint8_t> uncompressFile(const std::string& file_path);

    void compressFileWrite(const std::string& file_path, const std::string& write_file_path);
    void uncompressFileWrite(const std::string& file_path, const std::string& write_file_path);

   private:
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

    void generateCodes(const std::unique_ptr<HNode>& node, const HCode& prefix, HCodeMap& outCodes);

    std::vector<uint8_t> encode(HCodeMap& codes, const std::vector<uint8_t>& data);
    std::vector<uint8_t> decode(const HTree& tree, const std::vector<uint8_t>& data);
};

#endif // HCOMPRESSOR_H
