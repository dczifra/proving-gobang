#pragma once

#include "common.h"
#include "tree.h"
#include "board.h"
#include "math.h"

#include <map>

struct Args{
    int simulationNum = 10000;
    float cpuct = 1.0;
};

class MCTS{
public:
    MCTS(){

    }

    std::vector<int> get_action_prob(Board& b, int curPlayer){
        std::vector<int> v;

        TreeNode* act_node = states[b]; // TODO b not in states
        for(int i=0;i<args.simulationNum;i++){
            search(act_node, curPlayer, 0);
        }

        return v;
    }

    double search(TreeNode* node, int curPlayer, int round){
        // === If node is leaf, extend ===
        if(node == nullptr){
            // Extend Node
            //return node.init_val;
        }

        // === If terminal node, return end-value ===
        int end_val = node->end;
        if(end_val!=0) return -end_val;

        // === Calculate UCT Score, and choose argmax ===
        int action = 0;
        float max = 0.0;
        for(int i=0;i<ACTION_SIZE;i++){
            float Ns_child = 1.0+((node->children[i]==nullptr)?0:node->children[i]->Ns);
            float UCT = args.cpuct*node->probs[i]*std::sqrt(node->Ns)/(Ns_child);
            float score = node->Q[i]+UCT;

            if(max<score){
                max=score;
                action = i;
            }
        }

        double v = search(node->children[action], 1-curPlayer, 0);
        // === Update Ns, Q, Nsa ===
        // Nsa was updated in search, we are going upwards
        int Nsa = node->children[action]->Ns;
        float Qsa = node->Q[action];
        node->Q[action]=((Nsa-1)*Qsa+v)/(Nsa);
        node->Ns+=1;

        return -v;
    }


private:
    Args args;
    std::map<Board, TreeNode*> states;
    Heuristic heuristic;

};

bool operator<(const Board& b1, const Board& b2){
    return b1.white<b2.white || (b1.white == b2.white && b1.black<b2.black);
}