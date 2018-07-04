#include <stdint.h>
#include <fstream>
#include <cstring>
#include <iostream>

#include "BymlNodes.h"

int main(int argc, char const *argv[]) {
    std::string path;
    if(argc < 2) 
        return -1;
    else
        path = argv[1];

    std::ifstream fileis(path, std::ios::binary | std::ios::ate);
    std::streamsize size = fileis.tellg();
    fileis.seekg(0, std::ios::beg);

    uint8_t file[size];
    if(!fileis.read((char*)file, size)) {
        std::cout << "Could not open " << path << std::endl;
        return -1;
    }

    if(*(uint16_t*)file != LE_BYML_MAGIC) {
        std::cout << path << "is not Little Endian" << std::endl;
        return -2;
    }

    BymlHeader header;
    memcpy(&header, file, sizeof(header));

    if(file[header.name_tab_ofst] != STR_TAB_NODE_TYPE) {
        std::cout << "Name table error, expected string table type."
            << std::endl;
        return -3;
    }

    // Some testing code
    StringTabNode strTabNode(&file[header.name_tab_ofst]);
    std::cout << strTabNode[0x2A] << std::endl;

    DictNode root(&file[header.root_ofst], &file[header.name_tab_ofst]);
    std::cout << root.getNodeNameByIndex(0) << std::endl;

    return 0;
}
