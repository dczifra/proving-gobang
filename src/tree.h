#ifndef TREE_H_
#define TREE_H_

#include "common.h"

struct Board;

struct TreeNode{
    //const TreeNode* parent;
    TreeNode* children[ACTION_SIZE]={nullptr};
    float probs[ACTION_SIZE]={1.0/ACTION_SIZE};
    float Q[ACTION_SIZE]={0.0}; // No init is neccessary
    Board board;

    int Ns = 0;
    float v=0.0;
    int end = 0;
};

#endif // TREE_H_