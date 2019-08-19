#ifndef _BNODE_
#define _BNODE_

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>

#define t 3
#define SUCCESS 1
#define NOT_PRESENT 0
#define DATA_FILE "./datafile.dat"

struct keyValItem
{
public:
    char key[300];
    char val[512];

};

struct BNode
{
    bool isLeaf;                // true if node is Leaf
    int pos;                    // position of this node in FILE
    int count;                  // current number of records at each node
    int children[2 * t];        // array of positions of child-node in FILE
    keyValItem keys[2 * t - 1]; // array of records at each node
};



#endif