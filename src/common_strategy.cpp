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

void move_to_center(Board& act_board, int action, bool is_left){
    if(action == 7 || action == 47){
        if(is_left){
            if(((1ULL << 11) & act_board.black) || ((1ULL << 1) & act_board.black)) 0;// Move free
            else if((1ULL << 11) & act_board.white)  act_board.move(6,-1); // TODO: maybe cheating
            else act_board.move(11, -1);
        }
        else{
            act_board.node_type = OR;
            deactivate_line(act_board, 45);
        }
    }
    else if(action == 2 || action == 42){
        if(is_left){
            act_board.node_type = OR;
            deactivate_line(act_board, 5);
        }
        else{
            if((1ULL << 37) & act_board.black) 0;// Move free
            else if((1ULL << 37) & act_board.white)  act_board.move(47,-1);
            else act_board.move(37, -1);
        }
    }
}

Node* GeneralCommonStrategy::six_common_fields(Board& act_board, int action)
{
    // TODO: Ugly, burnt in variables
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;

    if(act_board.node_type == AND){
        int center = (action / ROW) * ROW + 2;
        act_board.forbidden_all ^= (1ULL << center - 1) | (1ULL << center) | (1ULL << center + 1);
        //act_board.move(is_left?7:42, -1);
        act_board.move(action, -1);
        act_board.forbidden_all &= ~side; // TODO !!!!!
    }
    else{
        // Center
        if(action == 7 || action == 47 || action == 2 || action == 42){
            act_board.move(action, 1);
            move_to_center(act_board, action, is_left);
            act_board.forbidden_all ^= (1ULL << action) | (1ULL << action - 1) | (1ULL << action + 1);
        }
        else if (action == 1 || action == 3 || action == 41 || action == 43 ||
                 action == 6 || action == 8 || action == 46 || action == 48){
            //int defender = (action / ROW)*ROW+2;
            int defender = (action / ROW) * ROW + 4 - (action % ROW);

            act_board.move(action, 1);
            act_board.move(defender, -1);
            //if(is_left) act_board.score_left = 1;
            //else act_board.score_right = 1;

            act_board.forbidden_all ^= (1ULL << action) | (1ULL << defender);
        }
        else{
            assert(0);
        }
    }
    return add_or_create(act_board);
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

Node *GeneralCommonStrategy::move_on_common(const Board &b, int action)
{
    Board act_board(b);
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    bool is_inner = (1ULL << action) & PNS::heuristic.forbidden_fields_inner;
    int &score = is_left ? act_board.score_left : act_board.score_right;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    int num_common_fields = __builtin_popcountll(act_board.forbidden_all & side & ~(b.white | b.black));

    if (num_common_fields == 6){
        return six_common_fields(act_board, action);
    }
    else if (num_common_fields == 4){
        if (act_board.node_type == AND){
            act_board.move(action, -1);
            // TODO: move to specific fields
        }
        else{
            int last_att_act = __builtin_ctzl(side & act_board.white);

            act_board.move(action, 1);
            int free = __builtin_ctzl(side & act_board.forbidden_all & ~(act_board.white | act_board.black));

            int center = (action / ROW) * ROW + 2;
            int opposite = (action / ROW) * ROW + 4 - (action % ROW);
            int next = ((1ULL << (action + 5)) & PNS::heuristic.forbidden_all) ? action + 5 : action - 5;
            //std::cout << next << std::endl;
            if ((action/ROW) == (last_att_act/ROW)){
                move_to_center(act_board, action, is_left);
                act_board.forbidden_all ^= (1ULL << (center - 1)) | (1ULL << center) | (1ULL << (center + 1));
                return add_or_create(act_board);
            }
            else if (action == center){
                int next0 = ((1ULL << (last_att_act + 5)) & PNS::heuristic.forbidden_all) ? last_att_act + 5 : last_att_act - 5;
                act_board.move(next0, -1);
            }
            else if (next == last_att_act){
                act_board.move(opposite, -1);
                //act_board.white |= (1ULL << (is_left?2:47));
                // Continue common for the last 2 nodes
                act_board.forbidden_all ^= ((1ULL << (action)) | (1ULL << opposite));
                return add_or_create(act_board);
            }
            else{
                act_board.move(opposite, -1);
                act_board.white |= (1ULL << (is_left?2:47));
                
                //act_board.move(center, -1);
                //board_int free = side & act_board.forbidden_all & ~(act_board.white | act_board.black);
                //if(is_inner) act_board.white |= free;
            }
        }
        act_board.forbidden_all &= ~side;
        return add_or_create(act_board);
    }
    else if (num_common_fields == 3){
        //board_int free_common = side & ~(act_board.white | act_board.black);
        //int childnum = __builtin_popcountll(free_common);
        // TODO: trouble, if 2 line not empty
        int last_att_act = __builtin_ctzl(side & act_board.white);
        int last_def_act = __builtin_ctzl(side & act_board.black);

        board_int white_side = act_board.white & side;
        board_int black_side = act_board.black;

        int center = (action / ROW) * ROW + 2;
        int opposite = (action / ROW) * ROW + 4 - (action % ROW);

        if (act_board.node_type == AND){
            act_board.move(action, -1);
            // TODO: move to specific fields
        }

        if (action == 7 || action == 47 || action == 2 || action == 42){
            act_board.move(action, 1);
            move_to_center(act_board, action, is_left);
        }
        else if (action == 1 || action == 41 || action == 6 || action == 46 ||
                 action == 3 || action == 43 || action == 8 || action == 48){
            act_board.move(action, 1);
            if (action == 6 || action == 8){
                act_board.move(action + 5, -1);
                // TODO if not valid
            }
            else if (action == 46 || action == 48){
                deactivate_line(act_board, action == 46 ? 45 : 35);
            }
            else if (action == 1 || action == 41 || action == 3 || action == 43){
                act_board.move(center, -1);
                if (!is_left)
                    act_board.white |= (1ULL << opposite);
            }
            else{
                assert(0);
            }
            act_board.forbidden_all &= act_board.get_valids();
        }
        else{
            assert(0);
        }
        act_board.forbidden_all &= ~side;
        return add_or_create(act_board);
    }
    if(num_common_fields == 1 || num_common_fields == 2){
        if (act_board.node_type == AND){
            act_board.move(action, -1);
            act_board.forbidden_all &= ~side;
            return add_or_create(act_board);
        }
        else{
            act_board.move(action, 1);
            move_to_center(act_board, action, is_left);
            act_board.forbidden_all ^= (1ULL << (action));
            return add_or_create(act_board);
        }
    }
    else{
        display(act_board, true, {action});
        display(act_board.forbidden_all, true);
        assert(0);
    }
}
