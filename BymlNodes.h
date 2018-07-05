#include <stdint.h>
#include <string>
#include <cstring>
#include <unordered_map>

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
class NodeTypeMismatch {};
class IHaventImplementedThisYetException {};

class Node
{
protected:
    uint8_t* start;
public:
    Node() {};
    Node(uint8_t* node_start);
    virtual ~Node();

    uint8_t* getStart() {return start;}
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


class ValueNode;

class DictNode : public Node {
private:
    int count = 0;
    StringTabNode* name_tab;
    bool nameTabNeedDelete = false;
    uint8_t* file;

    std::unordered_map<std::string, uint8_t*> ValueNodeMap;

    void initializeMaps();

public:
    DictNode(uint8_t* node_start, uint8_t* str_tab_start, uint8_t* file_ptr);
    DictNode(ValueNode* value_ptr);
    ~DictNode();

    ValueNode operator[](int i);
    ValueNode operator[](std::string name);

    int size() {return count;}
    std::string getNodeNameByIndex(int i);
    StringTabNode* getNameTab() {return name_tab;}
};


DictNode::DictNode(uint8_t* node_start, uint8_t* str_tab_start, uint8_t* file_ptr):
    Node(node_start), file(file_ptr) {
    name_tab = new StringTabNode(str_tab_start);
    nameTabNeedDelete = true;
    memcpy(&count, &start[1], 3);
}

class ValueNode : public Node {
private:
    int str_index = 0;
    uint8_t type;
    uint32_t* value;

    DictNode* parentDict;

public:
    ValueNode(uint8_t* node_start, DictNode* parentDict);
    ~ValueNode() {};

    void setValue(const uint32_t& x);

    uint32_t getValue() const {return *value;}
    uint8_t getType() const {return type;}
    DictNode* getParentDict() {return parentDict;}
    std::string name() {return (*(parentDict->getNameTab()))[str_index];}
};

ValueNode::ValueNode(uint8_t* node_start, DictNode* parent_dict_ptr):
    Node(node_start), parentDict(parent_dict_ptr) {
    memcpy(&str_index, node_start, 3);
    type = start[3];
    value = (uint32_t*)&start[4];
}

DictNode::DictNode(ValueNode* value_node_ptr) {
    if(value_node_ptr->getType() != DICT_NODE_TYPE) {
        throw NodeTypeMismatch();
    }
    DictNode* parentDict = value_node_ptr->getParentDict();
    name_tab = parentDict->name_tab;
    
    file = parentDict->file;
    start = &file[value_node_ptr->getValue()];
    memcpy(&count, &start[1], 3);
}

DictNode::~DictNode() {
    if(nameTabNeedDelete) {
        delete(name_tab);
    }
}

void DictNode::initializeMaps() {
    uint8_t* cur_node_start = start;
    // to be implemented
}

ValueNode DictNode::operator[](int i) {
    if(i < 0 || i >= count)
        throw OutOfBoundsException();
    uint8_t* node_start = &this->start[4 + i*8];
    uint8_t type = node_start[3];
    
    ValueNode ret = ValueNode(node_start, this);

    return ret;
}

ValueNode DictNode::operator[](std::string name) {
    // just search one by one for now
    // will probably implement some sort of hashmap solution later
    for (int i = 0; i < count; i++) {
        ValueNode curNode = (*this)[i];
        if(getNodeNameByIndex(i) == name) {
            return curNode;
        }
    }
    throw NodeNotFoundException();
}

std::string DictNode::getNodeNameByIndex(int i) {
    if(i < 0 || i >= count)
        throw OutOfBoundsException();
    uint8_t* node_start = &this->start[4 + i*8];
    int name_index = 0;
    memcpy(&name_index, node_start, 3);
    return (*name_tab)[name_index];
}



void ValueNode::setValue(const uint32_t& x) {
    memcpy(value, &x, sizeof(value));
}
