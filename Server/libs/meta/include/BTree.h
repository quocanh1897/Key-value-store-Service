#ifndef _BTREE_
#define _BTREE_

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include "BNode.h"

using namespace std;
struct BTree
{
    char fname[128];
    FILE *pFile;
    int root; 
    int nextPos; // next position for new node
};

BTree *BTreeInit(char *fname, bool existed);

BNode *splitChild(BTree* tree, BNode* x, int i, BNode* y);
void insertNonFull(BTree* tree, BNode *node, keyValItem* record);
void insert(BTree* tree, keyValItem* record);
void traverse(BTree* tree, int root);
keyValItem* search(BTree* tree, char *key);
keyValItem* recursiveSearch(BTree* tree, char *key, BNode* root);

bool removeFromTree(BTree* tree, char *key);
void merge(BTree* tree, BNode *node, int idx);
void borrowFromNext(BTree* tree, BNode *node, int idx);
void borrowFromPrev(BTree* tree, BNode *node, int idx);
void fill(BTree* tree, BNode *node, int idx);
keyValItem *getSucc(BTree* tree, BNode *node, int idx);
keyValItem *getPred(BTree* tree, BNode *node, int idx);

bool removeFromNonLeaf(BTree* tree, BNode *node, int idx);
bool removeFromLeaf (BTree* tree, BNode *node, int idx);
bool removeBNode(BTree* tree, BNode* node, char* k);
int findKey(BNode* node, char* k);

void set(BTree *tree, char *key, char *val);
char *get(BTree *tree, char *key);
void nodeCopy(BNode *a, BNode *b);
void printNode(BNode* node);

void BTreeClose(BTree* tree);
#endif
