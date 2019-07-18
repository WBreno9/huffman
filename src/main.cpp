#include <iostream>

#include "hcompressor.h"

int main(int argc, char** argv) {
    if (argc != 2) std::exit(EXIT_FAILURE);

    HCompressor compressor;
    compressor.compressFileWrite(argv[1], "out.huff");
    compressor.uncompressFileWrite("out.huff", "out2.huff");

    std::exit(EXIT_SUCCESS);
}
