#include <array>
#include <bitset>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <vector>

#include "heap.h"
#include "htree.h"
#include "hcompressor.h"

int main(int argc, char** argv) {
    if (argc != 2) std::exit(EXIT_FAILURE);

    HCompressor compressor;
    compressor.compress_file_write(argv[1], "out.huff");
    compressor.uncompress_file_write("out.huff", "out2.huff");

    std::exit(EXIT_SUCCESS);
}
