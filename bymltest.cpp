#include <stdint.h>
#include <fstream>
#include <cstring>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "BymlNodes.h"

void dictNodeDeepValueReplace(bool& foundReplace, DictNode& dict,
                                std::string name, const uint32_t& value) {
    // Go over nodes one by one, poc before I add map
    for(int i = 0; i < dict.size(); i++) {
        ValueNode curNode = dict[i];
        uint8_t curNodeType = curNode.getType();
        if(curNodeType == DICT_NODE_TYPE) {
            DictNode childDictNode(&curNode);
            dictNodeDeepValueReplace(foundReplace, childDictNode, name, value);
        } else if (curNode.name() == name) {
            switch(curNodeType) {
            case BOOL_NODE_TYPE:
            case INT_NODE_TYPE:
            case FLOAT_NODE_TYPE:
                curNode.setValue(value);
                foundReplace = true;
                std::cout << curNode.name() << " value set to 0x" << std::hex << value << std::endl;
                break;
            default:
                std::cout << "Unsupported ValueNode " << curNode.name() << " skipped" << std::endl;
                continue;
            }
        }
    }
}

int bymlFileValueReplace(std::string file_path, std::string node_name, const uint32_t& value) {
    std::ifstream fileis(file_path, std::ios::binary | std::ios::ate);
    std::streamsize size = fileis.tellg();
    fileis.seekg(0, std::ios::beg);

    uint8_t file[size];
    if(!fileis.read((char*)file, size)) {
        std::cout << "Could not open " << file_path << std::endl;
        fileis.close();
        return -1;
    }
    fileis.close();

    if(*(uint16_t*)file != LE_BYML_MAGIC) {
        std::cout << file_path << "is not Little Endian" << std::endl;
        return -2;
    }

    BymlHeader header;
    memcpy(&header, file, sizeof(header));

    if(file[header.name_tab_ofst] != STR_TAB_NODE_TYPE) {
        std::cout << "Name table error, expected string table type."
            << std::endl;
        return -3;
    }

    DictNode root(&file[header.root_ofst], &file[header.name_tab_ofst], file);
    bool foundReplace = false;
    dictNodeDeepValueReplace(foundReplace, root, node_name, value);
    if(foundReplace) {
        std::cout << "Done with " << file_path << std::endl;

        std::string backup_path = file_path + ".bak";
        if(access(backup_path.c_str(), F_OK) == -1) {
            if(std::rename(file_path.c_str(), backup_path.c_str()) != 0) {
                std::cout << "Failed to create backup for " << file_path << ", abort." << std::endl;
                return -4;
            }
        }

        std::ofstream fileos(file_path, std::ios::binary);
        if(!fileos.write((char*)file, size)) {
            std::cout << "Could not write " << file_path << std::endl;
            return -6;
        }
        fileos.close();
    }

    return 0;
}

const std::string PARAM_DICT_PATH = "splatparamdict.txt";

int main(int argc, char const *argv[]) {
    if(argc < 4) {
        std::cout << "Usage: " << std::endl;
        std::cout << argv[0] << " {byml file path} {node name} {new value for node}" << std::endl;
        std::cout << argv[0] << " {path to dir of byml files} {node name} {new value for node}" << std::endl;
        return 0;
    }

    std::string node_name = argv[2];
    // program only does one time access, so not mapping dict for now
    std::ifstream param_dict_is(PARAM_DICT_PATH);
    if(!param_dict_is.good()) {
        std::cout << "Param dictionary splatparamdict.txt not found." << std::endl;
    }
    std::string line;
    while(std::getline(param_dict_is, line)) {
        std::istringstream iss(line);
        std::string hashed_name, text_name;
        std::getline(iss, hashed_name, '\t');
        std::getline(iss, text_name);
        if(node_name == text_name) {
            node_name = hashed_name;
            std::cout << "Found " << text_name << " in dictionary as " << hashed_name << std::endl;
        }
    }

    uint32_t value;
    std::istringstream issi(argv[3]);
    int int_value;
    issi >> std::noskipws >> int_value;
    if(issi.eof() && !issi.fail()) {
        memcpy(&value, &int_value, sizeof(value));
    } else {
        std::istringstream issf(argv[3]);
        float float_value;
        issf >> std::noskipws >> float_value;
        if(issf.eof() && !issf.fail()) {
            memcpy(&value, &float_value, sizeof(value));
        } else {
            std::cout << "Failed to parse value: " << argv[3] << std::endl;
            return -5;
        }
    }
    std::cout << std::hex << "value is 0x" << value << std::endl;

    struct stat s;
    if(stat(argv[1], &s) == 0) {
        if(s.st_mode & S_IFDIR) {
            DIR *dir;
            struct dirent *ent;
            if((dir = opendir (argv[1])) != NULL) {
                while((ent = readdir (dir)) != NULL) {
                    if(ent->d_type != DT_REG)
                        continue;
                    std::string file_name = ent->d_name;
                    if(file_name.substr(file_name.length()-4) == ".bak")
                        continue;
                    std::string file_path = argv[1];
                    file_path = file_path + file_name;
                    bymlFileValueReplace(file_path, node_name, value);
                }
                closedir (dir);
            } else {
                std::cout << "Error with " << argv[1] << std::endl;
                return -1;
            }
        } else if(s.st_mode & S_IFREG) {
            bymlFileValueReplace(argv[1], node_name, value);
        } else {
            std::cout << "Error with " << argv[1] << std::endl;
            return -1;
        }
    } else {
        std::cout << "Error with " << argv[1] << std::endl;
        return -1;
    }

    std::cout << "Done" << std::endl;

    return 0;
}
