#pragma once

#include "common.h"
#include "board.h"

class Node;
class Args;

class CommonStrategy{
    virtual Node* move_on_common(const Board& b, int action, Args* args) = 0;
    virtual void move_on_common_defender(Board& next_state, int last_action) = 0;
};

class GeneralCommonStrategy: CommonStrategy{
public:
    GeneralCommonStrategy(){
        std::cout<<"GeneralCommonStrategy"<<std::endl;
    }

    virtual Node* move_on_common(const Board& b, int action, Args* args);
    virtual void move_on_common_defender(Board& next_state, int last_action){};

    Node* two_common_fields(Board& act_board, board_int side, int score, Args* args);
    Node* three_common_fields(Board& act_board, int action, Args* args);
    Node* four_common_fields(Board& act_board, int action, Args* args);
};