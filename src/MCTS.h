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
    MCTS(){}

    int play_random(Board& b, int player);
    float search(TreeNode* node, int curPlayer, int last_action);
    std::vector<int> get_action_prob(Board& b, int curPlayer);


private:
    Args args;
    std::map<Board, TreeNode*> states;
    Heuristic heuristic;

};
