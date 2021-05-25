#include "PNS.h"
#include "node.h"
#include "common_strategy.h"
#include "common.h"

#define has(action, board) ((1ULL << action) & board)

Node* CommonStrategy::add_or_create(const Board& board){
    PNSNode* node = tree->get_states(board);
    //display(board, true);
    if (node == nullptr){
        node = new PNSNode(board, tree->args);
        tree->add_board(board, node);
    }
    else{
        node->parent_num += 1;
    }
    return node;
}

void deactivate_line(Board& board, int line){
    board.white &= ~(1ULL << line);
    board.black |= (1ULL << line);
}

void activate_line(Board& board, int line){
    board.black &= ~(1ULL << line);
    board.white |= (1ULL << line);
}


Node *GeneralCommonStrategy::choose_from(const Board &board, std::vector<int> actions, NodeType type)
{
    int sum = 0;
    for (int action : actions){
        if (board.is_valid(action)){
            sum += 1;
        }
    }

    Node *node = new InnerNode(sum, type);
    int ind = 0;
    for (int action : actions){
        if (board.is_valid(action)){
            Board child(board);
            child.move(action, -1);
            node->children[ind] = add_or_create(child);
            ind += 1;
        }
    }
    tree->update_node(node);
    return node;
}

void move_2_line(Board& act_board, int action){
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    int def;
    if((action/ROW == 0) || (action/ROW == COL-2) ){
        def = action+ROW;
    }
    else{
        def = action - ROW;
    }
    act_board.move(def, -1);
    act_board.forbidden_all &= ~(1ULL << action);
    act_board.forbidden_all &= ~(1ULL << def);
}

void split_2_line(Board& act_board, int action){
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    board_int free = act_board.get_valids();

    act_board.forbidden_all &= ~(PNS::heuristic.forbidden_fields_inner & side);
    act_board.white |= (~PNS::heuristic.forbidden_fields_inner & side & free);
}

Node *GeneralCommonStrategy::move_on_common(const Board &b, int action){
    Board act_board(b);
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    bool is_inner = (1ULL << action) & PNS::heuristic.forbidden_fields_inner;
    int &score = is_left ? act_board.score_left : act_board.score_right;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    int num_common_fields = __builtin_popcountll(act_board.forbidden_all & side & ~(b.white | b.black));

    if(num_common_fields == 4){
        if(act_board.node_type == AND){
            act_board.move(action, -1);
            act_board.forbidden_all &= ~side;
        }
        else{
            act_board.move(action, 1);
            move_2_line(act_board, action);
            //split_2_line(act_board, action);
            act_board.forbidden_all &= ~side;
        }
        return add_or_create(act_board);   
    }
    if(num_common_fields == 2){
        assert(0);
        if(act_board.node_type == AND){
            act_board.move(action, -1);
            act_board.forbidden_all &= ~side;
        }
        else{
            act_board.move(action, 1);
            move_2_line(act_board, action);
        }
        return add_or_create(act_board);
    }
    else{
        display(act_board, true, {action});
        display(act_board.forbidden_all, true);
        assert(0);
    }
}
