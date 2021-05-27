#pragma once

#include "common.h"
#include "board.h"

class Node;
class Args;

class CommonStrategy{
public:
    CommonStrategy(PNS* tree_):tree(tree_){}
    virtual Node* move_on_common(const Board& b, int action) = 0;
    virtual void move_on_common_defender(Board& next_state, int last_action) = 0;

    Node* add_or_create(const Board& board);
protected:
    PNS* tree;
};

class GeneralCommonStrategy: protected CommonStrategy{
public:
    GeneralCommonStrategy(PNS* tree_) : CommonStrategy(tree_){
        std::cout<<"GeneralCommonStrategy"<<std::endl;
    }

    virtual Node* move_on_common(const Board& b, int action);
    virtual void move_on_common_defender(Board& next_state, int last_action){};
    Node* choose_from(const Board &board, std::vector<int> actions, NodeType type);

    Node* two_common_fields(Board& act_board, board_int side, int score);
    Node* three_common_fields(Board& act_board, int action);
    Node* four_common_fields(Board& act_board, int action);
    Node* six_common_fields(Board& act_board, int action);
    Node* move_free_and_give_up_others(Board& act_board, int action, board_int side);
    Node* answer_to_neighbouring_fields(Board& act_board, int action, board_int side);
    Node* no_move_and_get_others(Board& act_board, int action, board_int side);
};