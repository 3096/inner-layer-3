#include <stdint.h>
#include <fstream>
#include <cstring>
#include <iostream>

const uint16_t LE_BYML_MAGIC = 0x4259; // hex for "YB"

const uint8_t STR_NODE_TYPE         = 0xa0;
const uint8_t PATH_NODE_TYPE        = 0xa1;
const uint8_t ARRAY_NODE_TYPE       = 0xc0;
const uint8_t DICT_NODE_TYPE        = 0xc1;
const uint8_t STR_TAB_NODE_TYPE     = 0xc2;
const uint8_t PATH_TAB_NODE_TYPE    = 0xc3;
const uint8_t BOOL_NODE_TYPE        = 0xd0;
const uint8_t INT_NODE_TYPE         = 0xd1;
const uint8_t FLOAT_NODE_TYPE       = 0xd2;

typedef struct {
    uint16_t magic;
    uint16_t vers;
    uint32_t name_tab_ofst;
    uint32_t str_val_tab_ofst;
    uint32_t path_val_tab_ofst;    
} BymlHeader;

class OutOfBoundsException {};

class StringTabNode {
private:
    int count = 0;
    uint8_t* start;
public:
    StringTabNode(uint8_t* node_start);
    ~StringTabNode();
    
    std::string operator[](int i);
};

StringTabNode::StringTabNode(uint8_t* node_start) {
    start = node_start;
    memcpy(&count, &start[1], 3);
}

StringTabNode::~StringTabNode() {

}

std::string StringTabNode::operator[](int i) {
    if(i < 0 || i >= count)
        throw OutOfBoundsException();
    uint32_t str_val_ofst = *(uint32_t*)&start[4 + i*4];
    return (char*)&start[str_val_ofst];
}

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
        std::cout << "Name table is not of string table type." << std::endl;
        return -3;
    }
    StringTabNode strTabNode(&file[header.name_tab_ofst]);
    
    std::cout << strTabNode[0x2A] << std::endl;

    return 0;
}
