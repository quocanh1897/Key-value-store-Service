#include "BTree.h"
#include "BNode.h"
#include "Utils.h"
#include <iostream>

BTree *BTreeInit(char *fname, bool existed)
{
    //if not existed, create new db
    BTree *tree = (BTree *)malloc(sizeof(BTree));
    if (!existed)
    {
        strcpy(tree->fname, fname);
        tree->pFile = fopen(fname, "a+");
        tree->root = 0;
        tree->nextPos = 0;
        return tree;
    }

    //if there is datafile, load .config

    FILE *fin = fopen(".config", "a+");
    fseek(fin, 0, SEEK_SET);
    fread(tree, sizeof(BTree), 1, fin);
    fclose(fin);
    tree->pFile = fopen(fname, "r+");
    return tree;
    fclose(fin);
    
}

void traverse(BTree *tree, int root)
{
    /* There are n keys and n+1 children, travers through n keys */
    /* and first n children */
    if (root == -1)
        return;

    BNode *node = (BNode *)malloc(sizeof(BNode));

    diskRead(tree, node, root);

    printNode(node);

    for (int i = 0; i < 2 * t; i++)
    {
        traverse(tree, node->children[i]);
    }

    free(node);
}

/** 
 * Return the record corresponding to the key in target 
 * if fail, return NOT_PRESENT.
*/

keyValItem *search(BTree *tree, char *target)
{

    BNode *root = (BNode *)malloc(sizeof(BNode));
    diskRead(tree, root, tree->root);

    return recursiveSearch(tree, target, root);
}

keyValItem *recursiveSearch(BTree *tree, char *key, BNode *root)
{
    int i = 0;

    while (i < root->count && strcmp(key, root->keys[i].key) > 0)
        i++;

    if (i < root->count && strcmp(key, root->keys[i].key) == 0)
        return &root->keys[i];

    else if (root->isLeaf)
    {
        return NULL;
    }
    else
    {
        BNode *c_i = (BNode *)malloc(sizeof(BNode));
        diskRead(tree, c_i, root->children[i]);
        return recursiveSearch(tree, key, c_i);
    }
}

/** 
 * Insert a new record into node (this node must be non-full when function is called)
*/
void insert(BTree *tree, keyValItem *k)
{
    /* If tree is empty */
    if (tree->nextPos == NULL)
    {
        /* Allocate memory for root */
        tree->root = tree->nextPos;
        BNode *rootNode = (BNode *)malloc(sizeof(BNode));
        nodeInit(rootNode, true, tree);
        rootNode->keys[0] = *k;
        rootNode->count++;

        //START TO WRITE
        diskWrite(tree, rootNode, rootNode->pos);
        free(rootNode);
    }
    else
    {
        BNode *root = (BNode *)malloc(sizeof(BNode));
        diskRead(tree, root, tree->root);
        int r = tree->root;
        /* If root is full, then tree grows in height */
        if (root->count == 2 * t - 1)
        {
            /* Allocate memory for new root (s) */
            BNode *s = (BNode *)malloc(sizeof(BNode));
            nodeInit(s, false, tree);

            tree->root = s->pos;
            /* Make old root as child of new root (s)*/
            s->children[0] = r;

            /* Split the old root and move 1 key to the new root (s)*/
            BNode *temp = splitChild(tree, s, 0, root);

            if (strcmp(temp->keys[0].key, k->key) <= 0)
            {
                BNode *tmp = s;
                s = temp;
                free(tmp);
            }
            else
            {
                free(temp);
            }

            /**
             * New root (s) has two children now.
             * Decide which of the two children 
             * is going to have new key 
            */
            // int i = 0;
            // if (strcmp(s->keys[i].key, k->key) < 0)
            //     i++;

            // BNode *c_i = (BNode*)malloc(sizeof(BNode));
            // diskRead(tree, c_i, s->children[i]);
            insertNonFull(tree, s, k);

            // cout << "_______node c_i" << endl;
            // printNode(root);
            // cout << "_______________ " << endl;

            // tree->root = s->pos;

            // diskWrite(tree, root, root->pos);

            free(s);
        }
        else // If root is not full, call insertNonFull for root
            insertNonFull(tree, root, k);

        free(root);
    }
}

bool removeBNode(BTree *tree, BNode *node, char *k)
{
    if (node->count == -1)
    {
        tree->root = 0;
        tree->nextPos = 0;
        BNode *node = (BNode *)malloc(sizeof(BNode));
        nodeInit(node, true, tree);
        diskWrite(tree, node, 0);
        return true;
    }

    int idx = findKey(node, k);
    bool res;
    // The key to be removed is present in this node
    if (idx < node->count && strcmp(node->keys[idx].key, k) == 0)
    {

        // If the node is a leaf node - removeFromLeaf is called
        // Otherwise, removeFromNonLeaf function is called
        if (node->isLeaf)
            res = removeFromLeaf(tree, node, idx);
        else
            res = removeFromNonLeaf(tree, node, idx);

        // diskWrite(tree, node, node->pos);
    }
    else
    {

        if (node->isLeaf)
        {
            free(node);
            return false;
        }

        // The key to be removed is present in the sub-tree rooted with this node
        // The flag indicates whether the key is present in the sub-tree rooted
        // with the last child of this node
        bool flag = ((idx == node->count) ? true : false);

        // If the child where the key is supposed to exist has less that t keys,
        // we fill that child

        BNode *c_i = (BNode *)malloc(sizeof(BNode));
        diskRead(tree, c_i, node->children[idx]);

        if (c_i->count < t)
            fill(tree, node, idx);

        // If the last child has been merged, it must have merged with the previous
        // child and so we recurse on the (idx-1)th child. Else, we recurse on the
        // (idx)th child which now has atleast t keys
        if (flag && idx > node->count)
        {
            BNode *sibling = (BNode *)malloc(sizeof(BNode));
            diskRead(tree, sibling, node->children[idx - 1]);
            free(node);
            res = removeBNode(tree, c_i, k);
        }
        else
        {
            BNode *c_i = (BNode *)malloc(sizeof(BNode));
            diskRead(tree, c_i, node->children[idx]);
            free(node);
            res = removeBNode(tree, c_i, k);
        }
    }
    return res;
}

bool removeFromTree(BTree *tree, char *key)
{

    BNode *root = (BNode *)malloc(sizeof(BNode));
    diskRead(tree, root, tree->root);
    return removeBNode(tree, root, key);
}

void insertNonFull(BTree *tree, BNode *node, keyValItem *k)
{
    int i = node->count - 1;
    int kt = findKey(node, k->key);
    /* if k is already existed then override the corresponding record*/
    if (kt < node->count && strcmp(node->keys[kt].key, k->key) == 0)
    {
        node->keys[kt] = *k;
        diskWrite(tree, node, node->pos);
        return;
    }

    if (node->isLeaf == true)
    {
        // a) Finds the location of new key to be inserted
        // b) Moves all greater keys to one place ahead
        while (i >= 0 && strcmp(node->keys[i].key, k->key) > 0)
        {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = *k;
        node->count++;
        diskWrite(tree, node, node->pos);
    }
    else
    {
        while (i >= 0 && strcmp(node->keys[i].key, k->key) > 0)
            i--;

        BNode *c_i = (BNode *)malloc(sizeof(BNode));

        diskRead(tree, c_i, node->children[i + 1]);

        if (c_i->count == (2 * t - 1))
        {
            BNode *temp = splitChild(tree, node, i + 1, c_i);

            if (strcmp(node->keys[i + 1].key, k->key) < 0)
            {
                BNode *tmp = c_i;
                c_i = temp;
                if (!temp)
                    free(tmp);
            }
            else
            {
                free(temp);
            }
        }

        // diskRead(tree, c_i, node->children[i+1]);
        insertNonFull(tree, c_i, k);
        free(c_i);
    }
}
BNode *splitChild(BTree *tree, BNode *x, int i, BNode *y)
{
    BNode *z = (BNode *)malloc(sizeof(BNode));
    nodeInit(z, y->isLeaf, tree);
    z->count = t - 1;

    /* Copy the last (t-1) keys of y to z */
    for (int j = 0; j < t - 1; j++)
        z->keys[j] = y->keys[j + t];

    /* Copy the last t children of y to z */
    if (!y->isLeaf)
    {
        for (int j = 0; j < t; j++)
            z->children[j] = y->children[j + t];
    }

    /* Reduce the number of keys in y */
    y->count = t - 1;

    /* Since this node is going to have a new child, */
    /* create space of new child */
    for (int j = x->count; j >= i + 1; j--)
        x->children[j + 1] = x->children[j];

    /* Link the new child to this node */
    x->children[i + 1] = z->pos;

    /* A key of y will move to this node. Find location of */
    /* new key and move all greater keys one space ahead */
    for (int j = x->count - 1; j >= i; j--)
        x->keys[j + 1] = x->keys[j];

    /* Copy the middle key of y to this node */
    x->keys[i] = y->keys[t - 1];

    /* Increment x->count of keys in this node */
    x->count++;

    diskWrite(tree, x, x->pos);
    diskWrite(tree, y, y->pos);
    diskWrite(tree, z, z->pos);
    return z;
}

/**
 * Returns the index of the first key that is
 * greater than or equal to k
 */

int findKey(BNode *node, char *k)
{
    int idx = 0;
    while (idx < node->count && strcmp(node->keys[idx].key, k) < 0)
        ++idx;
    return idx;
}

// Remove the idx-th key from this node - which is a Leaf node
bool removeFromLeaf(BTree *tree, BNode *node, int idx)
{

    // Move all the keys after the idx-th pos one place backward
    for (int i = idx + 1; i < node->count; ++i)
        node->keys[i - 1] = node->keys[i];

    // Reduce the count of keys
    node->count--;
    diskWrite(tree, node, node->pos);
    free(node);
    return false;
}

// Remove the idx-th key from this node - which is a non-Leaf node
bool removeFromNonLeaf(BTree *tree, BNode *node, int idx)
{

    char *k = node->keys[idx].key;
    bool res;

    BNode *child = (BNode *)malloc(sizeof(BNode));
    BNode *sibling = (BNode *)malloc(sizeof(BNode));

    diskRead(tree, child, node->children[idx]);
    diskRead(tree, sibling, node->children[idx - 1]);
    // If the child that precedes k (children[idx]) has atleast t keys,
    // find the predecessor 'pred' of k in the subtree rooted at
    // children[idx]. Replace k by pred. Recursively delete pred
    // in children[idx]
    if (child->count >= t)
    {
        keyValItem *pred = getPred(tree, node, idx);
        node->keys[idx] = *pred;
        res = removeBNode(tree, child, pred->key);
    }

    // If the child children[idx] has less that t keys, examine children[idx+1].
    // If children[idx+1] has atleast t keys, find the successor 'succ' of k in
    // the subtree rooted at children[idx+1]
    // Replace k by succ
    // Recursively delete succ in children[idx+1]
    else if (sibling->count >= t)
    {
        keyValItem *succ = getSucc(tree, node, idx);
        node->keys[idx] = *succ;
        res = removeBNode(tree, sibling, succ->key);
    }

    // If both children[idx] and children[idx+1] has less that t keys,merge k and all of children[idx+1]
    // into children[idx]
    // Now children[idx] contains 2t-1 keys
    // Free children[idx+1] and recursively delete k from children[idx]
    else
    {
        merge(tree, node, idx);
        res = removeBNode(tree, child, k);
    }

    diskWrite(tree, child, child->pos);
    diskWrite(tree, sibling, sibling->pos);
    // free(child);
    // free(sibling);
    return res;
}

// A function to get predecessor of keys[idx]
keyValItem *getPred(BTree *tree, BNode *node, int idx)
{
    // Keep moving to the right most node until we reach a leaf

    BNode *curr = (BNode *)malloc(sizeof(BNode));
    diskRead(tree, curr, node->children[idx]);
    while (!curr->isLeaf)
        diskRead(tree, curr, curr->children[curr->count]);

    // Return the last key of the leaf
    return &(curr->keys[curr->count - 1]);
}

keyValItem *getSucc(BTree *tree, BNode *node, int idx)
{

    // Keep moving the left most node starting from C[idx+1] until we reach a leaf
    BNode *curr = (BNode *)malloc(sizeof(BNode));
    diskRead(tree, curr, node->children[idx]);
    while (!curr->isLeaf)
        diskRead(tree, curr, curr->children[0]);

    // Return the first key of the leaf
    return &(curr->keys[0]);
}

// A function to fill child children[idx] which has less than t-1 keys
void fill(BTree *tree, BNode *node, int idx)
{
    BNode *c_prev = (BNode *)malloc(sizeof(BNode));
    BNode *c_succ = (BNode *)malloc(sizeof(BNode));
    diskRead(tree, c_prev, node->children[idx - 1]);
    diskRead(tree, c_succ, node->children[idx + 1]);

    // If the previous child(children[idx-1]) has more than t-1 keys, borrow a key
    // from that child
    if (idx != 0 && c_prev->count >= t)
        borrowFromPrev(tree, node, idx);

    // If the next child(C[idx+1]) has more than t-1 keys, borrow a key
    // from that child
    else if (idx != node->count && c_succ->count >= t)
        borrowFromNext(tree, node, idx);

    // Merge C[idx] with its sibling
    // If C[idx] is the last child, merge it with with its previous sibling
    // Otherwise merge it with its next sibling
    else
    {
        if (idx != node->count)
            merge(tree, node, idx);
        else
            merge(tree, node, idx - 1);
    }
    free(c_prev);
    free(c_succ);
    return;
}

// A function to borrow a key from children[idx-1] and insert it
// into children[idx]
void borrowFromPrev(BTree *tree, BNode *node, int idx)
{
    BNode *child = (BNode *)malloc(sizeof(BNode));
    BNode *sibling = (BNode *)malloc(sizeof(BNode));

    diskRead(tree, child, node->children[idx]);
    diskRead(tree, sibling, node->children[idx - 1]);

    // The last key from C[idx-1] goes up to the parent and key[idx-1]
    // from parent is inserted as the first key in C[idx]. Thus, the  loses
    // sibling one key and child gains one key

    // Moving all key in C[idx] one step ahead
    for (int i = child->count - 1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];

    // If C[idx] is not a leaf, move all its child pointers one step ahead
    if (!child->isLeaf)
    {
        for (int i = child->count; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    // Setting child's first key equal to keys[idx-1] from the current node
    child->keys[0] = node->keys[idx - 1];

    // Moving sibling's last child as C[idx]'s first child
    if (!node->isLeaf)
        child->children[0] = sibling->children[sibling->count];

    // Moving the key from the sibling to the parent
    // This reduces the number of keys in the sibling
    node->keys[idx - 1] = sibling->keys[sibling->count - 1];

    child->count += 1;
    sibling->count -= 1;

    diskWrite(tree, node, node->pos);
    diskWrite(tree, child, child->pos);
    diskWrite(tree, sibling, sibling->pos);
    free(child);
    free(sibling);
    return;
}

// A function to borrow a key from the C[idx+1] and place
// it in C[idx]
void borrowFromNext(BTree *tree, BNode *node, int idx)
{

    BNode *child = (BNode *)malloc(sizeof(BNode));
    BNode *sibling = (BNode *)malloc(sizeof(BNode));

    diskRead(tree, child, node->children[idx]);
    diskRead(tree, sibling, node->children[idx + 1]);

    // keys[idx] is inserted as the last key in C[idx]
    child->keys[(child->count)] = node->keys[idx];

    // Sibling's first child is inserted as the last child
    // into C[idx]
    if (!(child->isLeaf))
        child->children[(child->count) + 1] = sibling->children[0];

    //The first key from sibling is inserted into keys[idx]
    node->keys[idx] = sibling->keys[0];

    // Moving all keys in sibling one step behind
    for (int i = 1; i < sibling->count; ++i)
        sibling->keys[i - 1] = sibling->keys[i];

    // Moving the child pointers one step behind
    if (!sibling->isLeaf)
    {
        for (int i = 1; i <= sibling->count; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }

    // Increasing and decreasing the key count of C[idx] and C[idx+1]
    // respectively
    child->count += 1;
    sibling->count -= 1;

    diskWrite(tree, node, node->pos);
    diskWrite(tree, child, child->pos);
    diskWrite(tree, sibling, sibling->pos);

    free(child);
    free(sibling);
    return;
}

// A function to merge C[idx] with C[idx+1]
// C[idx+1] is freed after merging
void merge(BTree *tree, BNode *node, int idx)
{

    BNode *child = (BNode *)malloc(sizeof(BNode));
    BNode *sibling = (BNode *)malloc(sizeof(BNode));

    diskRead(tree, child, node->children[idx]);
    diskRead(tree, sibling, node->children[idx + 1]);

    // Pulling a key from the current node and inserting it into (t-1)th
    // position of C[idx]
    child->keys[t - 1] = node->keys[idx];

    // Copying the keys from C[idx+1] to C[idx] at the end
    for (int i = 0; i < sibling->count; ++i)
        child->keys[i + t] = sibling->keys[i];

    // Copying the child pointers from C[idx+1] to C[idx]
    if (!child->isLeaf)
    {
        for (int i = 0; i <= sibling->count; ++i)
            child->children[i + t] = sibling->children[i];
    }

    // Moving all keys after idx in the current node one step before -
    // to fill the gap created by moving keys[idx] to C[idx]
    for (int i = idx + 1; i < node->count; ++i)
        node->keys[i - 1] = node->keys[i];

    // Moving the child pointers after (idx+1) in the current node one
    // step before
    for (int i = idx + 2; i <= node->count; ++i)
        node->children[i - 1] = node->children[i];

    // Updating the key count of child and the current node
    child->count += sibling->count + 1;
    node->count--;

    // Freeing the memory occupied by sibling
    diskWrite(tree, node, node->pos);
    diskWrite(tree, child, child->pos);
    diskWrite(tree, sibling, sibling->pos);
    return;
}

void nodeCopy(BNode *a, BNode *b)
{
    a->isLeaf = b->isLeaf;
    a->pos = b->pos;
    a->count = b->count;
    int i;
    for (i = 0; i < (2 * t - 1); i++)
    {
        a->keys[i] = b->keys[i];
        a->children[i] = b->children[i];
    }
    a->children[i] = b->children[i];
}

void printNode(BNode *node)
{

    cout << "Keys: " << endl;
    for (int i = 0; i < node->count; i++)
    {
        cout << " " << node->keys[i].key << ": " << node->keys[i].val << "  ";
    }
    cout << "\nChild - position: " << endl;
    for (int i = 0; i <= node->count; i++)
    {
        cout << " " << node->children[i];
    }
    cout << endl;
}

void set(BTree *tree, char *key, char *val)
{
    keyValItem item;
    strcpy(item.key, key);
    strcpy(item.val, val);
    insert(tree, &item);
}

char *get(BTree *tree, char *key)
{

    keyValItem *temp = search(tree, key);
    if (temp)
        return temp->val;
    return "NULL";
}

void BTreeClose(BTree *tree)
{
    FILE *f = fopen(".config", "w");
    fseek(f, 0, SEEK_SET);
    fwrite(tree, sizeof(BTree), 1, f);
    fclose(f);

    if (tree->pFile != NULL)
        fclose(tree->pFile);
    free(tree);
}