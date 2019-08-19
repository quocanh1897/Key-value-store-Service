#ifndef _UTILS_
#define _UTILS_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "BNode.h"
#include "BTree.h"


void nodeInit(BNode *node, bool isLeaf, BTree *tree) {
    
    node->isLeaf = isLeaf;
    node->count = 0;
    node->pos = tree->nextPos;
    tree->nextPos++;
    
    for(int i = 0; i < 2*t; i++) {
        node->children[i] = -1;
    }
    
}

void printFunc(keyValItem *k) {
    
    if(!k) {
        cout << "Record Not Found!!\n\n";
        return;
    }
    
    cout << "\nKey: \n" << k->key;
    cout << "Value: \n" << k->val;
    
}

void diskWrite(BTree* ptrBTree, BNode* p, int pos) {
    // cout << "SIZE of a node: " << sizeof(BNode) << endl;
    if(pos == -1) {
        pos = ptrBTree->nextPos++;
    }

    fseek(ptrBTree->pFile, pos * sizeof(BNode), 0);
    fwrite(p, sizeof(BNode), 1, ptrBTree->pFile);
    fsync(fileno(ptrBTree->pFile));
    
}

void diskRead(BTree* ptrBTree, BNode* p, int pos) {
    
    fseek(ptrBTree->pFile, pos * sizeof(BNode), 0);
    fread(p, sizeof(BNode), 1, ptrBTree->pFile);

}

bool checkExist(char *fname){
    if (FILE *temp = fopen(fname, "r")){
        fclose(temp);
        return true;
    }
    else{
        return false;
    }
}

#endif