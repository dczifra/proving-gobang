#ifndef TREE_H_
#define TREE_H_

#include "common.h"
#include "board.h"
//struct Board;

struct TreeNode{
    TreeNode():children(){
        for(int i=0;i<ACTION_SIZE;i++){
            probs[i]=1.0/ACTION_SIZE;
        }
    }
    //const TreeNode* parent;
    TreeNode* children[ACTION_SIZE];
    std::array<float, ACTION_SIZE> probs;
    float Q[ACTION_SIZE]={0.0}; // No init is neccessary
    Board board;

    int Ns = 0;
    int end = 0;
};

#endif // TREE_H_