#include <stdint.h>
#include <string>
#include <cstring>

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
    uint32_t unk;
    uint32_t root_ofst;    
} BymlHeader;

class OutOfBoundsException {};

class NodeNotFoundException {};

class IHaventImplementedThisYetException {};

class Node
{
protected:
    uint8_t* start;
public:
    Node(uint8_t* node_start);
    ~Node();
};

Node::Node(uint8_t* node_start) {
    start = node_start;
}

Node::~Node() {

}

class StringTabNode : public Node {
private:
    int count = 0;
    
public:
    StringTabNode(uint8_t* node_start);
    ~StringTabNode();
    
    std::string operator[](int i);
};


StringTabNode::StringTabNode(uint8_t* node_start):
    Node(node_start) {
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


template <typename T>
class ValueNode : public Node {
private:
    int str_index = 0;
    const uint8_t* type;
    uint32_t* value;

    StringTabNode* name_tab;

public:
    ValueNode(uint8_t* node_start, StringTabNode* name_tab_ptr);
    ~ValueNode() {};

    T& operator=(const T& x);

    std::string name() {return (*name_tab)[str_index];}
};

template <typename T>
ValueNode<T>::ValueNode(uint8_t* node_start, StringTabNode* name_tab_ptr):
    Node(node_start), name_tab(name_tab_ptr) {
    memcpy(&str_index, node_start, 3);
    type = &start[3];
    value = (uint32_t*)&start[4];
};

template <typename T>
T& ValueNode<T>::operator=(const T& x) {
    memcpy(value, x, sizeof(value));
    return *(T*)value;
}



class DictNode : public Node {
private:
    int count = 0;
    StringTabNode* name_tab;
    bool nameTabNeedDelete = false;

public:
    DictNode(uint8_t* node_start, uint8_t* str_tab_start);
    DictNode(uint8_t* node_start, StringTabNode* name_tab_ptr);
    ~DictNode();

    Node operator[](int i);
    Node operator[](std::string name);

    int size() {return count;}
    std::string getNodeNameByIndex(int i);
};

DictNode::DictNode(uint8_t* node_start, uint8_t* str_tab_start):
    Node(node_start) {
    memcpy(&count, &start[1], 3);
    name_tab = new StringTabNode(str_tab_start);
    nameTabNeedDelete = true;
}

DictNode::DictNode(uint8_t* node_start, StringTabNode* name_tab_ptr):
    Node(node_start), name_tab(name_tab_ptr) {
    memcpy(&count, &start[1], 3);
}

DictNode::~DictNode() {
    if(nameTabNeedDelete) {
        delete(name_tab);
    }
}

Node DictNode::operator[](int i) {
    if(i < 0 || i >= count)
        throw OutOfBoundsException();
    uint8_t* node_start = &this->start[4 + i*4];
    uint8_t type = node_start[3];
    
    Node ret = NULL;
    switch(type) {
    case STR_NODE_TYPE:
    case PATH_NODE_TYPE:
        ret = ValueNode<uint32_t>(node_start, name_tab);
        break;
    case ARRAY_NODE_TYPE:
        throw IHaventImplementedThisYetException();
        break;
    case DICT_NODE_TYPE:
        ret = DictNode(node_start, name_tab);
        break;
    case STR_TAB_NODE_TYPE:
        ret = StringTabNode(node_start);
        break;
    case PATH_TAB_NODE_TYPE:
        throw IHaventImplementedThisYetException();
        break;
    case BOOL_NODE_TYPE:
        ret = ValueNode<bool>(node_start, name_tab);
        break;
    case INT_NODE_TYPE:
        ret = ValueNode<int>(node_start, name_tab);
        break;
    case FLOAT_NODE_TYPE:
        ret = ValueNode<float>(node_start, name_tab);
        break;
    default:
        throw IHaventImplementedThisYetException();
    }

    return ret;
}

Node DictNode::operator[](std::string name) {
    // just search one by one for now
    // will probably implement some sort of hashmap solution later
    for (int i = 0; i < count; i++) {
        Node curNode = (*this)[i];
        if(getNodeNameByIndex(i) == name) {
            return curNode;
        }
    }
    throw NodeNotFoundException();
}

std::string DictNode::getNodeNameByIndex(int i) {
    if(i < 0 || i >= count)
        throw OutOfBoundsException();
    uint8_t* node_start = &this->start[4 + i*4];
    int name_index = 0;
    memcpy(&name_index, node_start, 3);
    return (*name_tab)[name_index];
}
