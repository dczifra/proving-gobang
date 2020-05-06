#pragma once

#include "common.h"
#include "tree.h"
#include "board.h"
#include "math.h"
#include "heuristic.h"

#include <map>

struct Args{
    int simulationNum = 100000;
    float cpuct = 1.0;
};

class MCTS{
public:
    MCTS(){}

    int play_random(Board& b, int player);
    std::vector<int> get_action_prob(Board& b, int curPlayer);
    float search(TreeNode* node, int curPlayer, int last_action);
    void extend_node(TreeNode*& node, const Board& board, int last_action);

private:
    Args args;
    std::map<Board, TreeNode*> states;
    Heuristic heuristic;

};
