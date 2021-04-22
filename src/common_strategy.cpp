#include "PNS.h"
#include "node.h"
#include "common_strategy.h"
#include "common.h"

#define has(action,board) ((1ULL << action) & board)

Node* CommonStrategy::add_or_create(const Board& board){
    PNSNode* node = tree->get_states(board);
    //display(board, true);
    if(node == nullptr){
        node = new PNSNode(board, tree->args);
        tree->add_board(board, node);
    }
    else{
        node->parent_num +=1;
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

Node* GeneralCommonStrategy::six_common_fields(Board& act_board, int action){
    // TODO: Ugly, burnt in variables
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;

    if(act_board.node_type == AND){
        int center = (action/ROW)*ROW+2;
        act_board.forbidden_all ^= (1ULL << center-1) | (1ULL << center) | (1ULL << center+1);
        //act_board.move(is_left?7:42, -1);
        act_board.move(action, -1);
        act_board.forbidden_all &= ~side; // TODO !!!!!
    }
    else{
        // Center
        if(action == 7 || action == 47){
            act_board.move(action, 1);
            if(is_left){
                act_board.move(11,-1);
            }
            else{
                act_board.node_type = OR;
                deactivate_line(act_board, 45);
            }
            act_board.forbidden_all ^= (1ULL << action) | (1ULL << action-1) | (1ULL << action+1);
        }
        else if(action == 2 || action == 42){
            act_board.move(action, 1);
            if(is_left){
                act_board.node_type = OR;
                deactivate_line(act_board, 5);
            }
            else{
                act_board.move(37,-1);
            }
            act_board.forbidden_all ^= (1ULL << action) | (1ULL << action-1) | (1ULL << action+1);
        }
        else if(action == 1 || action == 3 || action == 41 || action == 43 ||
                action == 6 || action == 8 || action == 46 || action == 48){
            int defender = (action / ROW)*ROW+2;
            int opposite = (action/ROW)*ROW+4-(action%ROW);

            act_board.move(action, 1);
            act_board.move(defender, -1);
            int give_up1, other_center;
            if(has(action, PNS::heuristic.forbidden_fields_inner)){
                give_up1 = is_left?action-5:action+5;
                other_center = (give_up1/ROW)*ROW+2;
                act_board.move(give_up1, 1);
                act_board.move(other_center, -1);
            }
            else{
                give_up1 = is_left?action+5:action-5;
                other_center = (give_up1/ROW)*ROW+2;
                if(is_left) act_board.score_left = 1;
                else act_board.score_right = 1;
            }
            act_board.forbidden_all ^= (1ULL << action) | (1ULL << defender);
            act_board.forbidden_all ^= (1ULL << give_up1) | (1ULL << other_center);
        }
        else{
            assert(0);
        }
    }
    return add_or_create(act_board);
}

Node* GeneralCommonStrategy::choose_from(const Board &board, std::vector<int> actions, NodeType type){
    int sum = 0;
    for(int action: actions){
        if(board.is_valid(action)){
            sum +=1;
        }
    }

    Node* node = new InnerNode(sum, type);
    int ind = 0;
    for(int action: actions){
        if(board.is_valid(action)){
            Board child(board);
            child.move(action, -1);
            node->children[ind] = add_or_create(child);
            ind+=1;
        }
    }
    tree->update_node(node);
    return node;
}

Node* GeneralCommonStrategy::move_on_common(const Board& b, int action){
    Board act_board(b);
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    bool is_inner = (1ULL << action) & PNS::heuristic.forbidden_fields_inner;
    int& score = is_left ? act_board.score_left : act_board.score_right;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    int num_common_fields = __builtin_popcountll(act_board.forbidden_all & side & ~(b.white | b.black));

    if(num_common_fields == 6){
        return six_common_fields(act_board, action);
    }
    else if(num_common_fields == 2){
        if(act_board.node_type == AND){
            act_board.move(action, -1);
            // TODO: move to specific fields
        }
        else{
            act_board.move(action, 1);
            int free = __builtin_ctzl(side & act_board.forbidden_all & ~(act_board.white | act_board.black));
            
            if(score == 1){
                score=0;
                // If one of the 7 line is covered, can move free
                int center = is_left?12:37;
                int opposite = (action/ROW)*ROW+4-(action%ROW);
                if(is_inner & (has(center-1, act_board.black) || has(center+1, act_board.black))){
                    // Move free
                    //act_board.white |= (1ULL << free);
                    if(has(center-1, act_board.black) && has(center+1, act_board.black)){
                        // move free
                    }
                    else if(has(opposite, act_board.black)){
                        // move free
                    }
                    else{
                        // === CHEAT ===:
                        //act_board.white |= (1ULL << free);
                    }
                }
                else if(!is_inner){
                    //act_board.move(free, -1)
                    // Cannot move, but does not have to cover the side 5
                    int center = (action/ROW)*ROW+2;
                    int line = is_left?5:40;
                    line += (action>center?5:-5);
                    act_board.white &= ~(1ULL << line);
                    act_board.black |= (1ULL << line);
                    act_board.node_type = OR;
                }
                else{
                    if(has(opposite, act_board.black)){
                        // move free
                    }
                    else{
                        act_board.forbidden_all &= ~side;
                        std::vector<int> def_choices = {center-1, center+1, opposite, free};

                        int i=0;
                        Node* node = new InnerNode(4, AND);
                        for(auto def: def_choices){
                            Board child(act_board);
                            if(child.is_valid(def)){
                                child.move(def, -1);
                                if(def != opposite || def != free){
                                    // === CHEAT ===
                                    //child.white |= (1ULL << free);
                                }
                            }
                            else child.move(free, -1);
                            node->children[i++] = add_or_create(child);
                        }
                        tree->update_node(node);
                        return node;
                    }
                }
            }
            else{
                act_board.forbidden_all &= ~side;
                Board child1(act_board);
                Board child2(act_board);
                if(!is_inner){
                    // Cannot answer, but gets the other field
                    act_board.node_type = OR;
                }
                else{
                    int def = is_left?action+5:action-5;
                    if(has(def, act_board.black)){
                        // Move free
                    }
                    else if(!has(def, act_board.white)){
                        act_board.move(def, -1);
                    }
                    else{
                        act_board.move(free, -1);
                    }
                }
                
                if(!is_inner){
                    Node* node = new InnerNode(3,OR);
                    node->children[0] = add_or_create(act_board);

                    child1.white &= ~(1ULL << (is_left?0:45));
                    child1.black |= (1ULL << (is_left?0:45));
                    //child1.white |= (1ULL << free);
                    child1.node_type = OR;
                    node->children[1] = add_or_create(child1);

                    child2.white &= ~(1ULL << (is_left?10:35));
                    child2.black |= (1ULL << (is_left?10:35));
                    //child2.white |= (1ULL << free);
                    child2.node_type = OR;
                    node->children[2] = add_or_create(child2);

                    tree->update_node(node);
                    return node;
                }
            }
        }
        act_board.forbidden_all &= ~side;
        return add_or_create(act_board);
    }
    else if(num_common_fields == 5 || num_common_fields == 4 || num_common_fields == 3){
        //board_int free_common = side & ~(act_board.white | act_board.black);
        //int childnum = __builtin_popcountll(free_common);
        // TODO: trouble, if 2 line not empty
        int last_att_act = __builtin_ctzl(side & act_board.white);
        int last_def_act = __builtin_ctzl(side & act_board.black);

        board_int white_side = act_board.white & side;
        board_int black_side = act_board.black;

        int center = (action/ROW)*ROW+2;
        int opposite = (action/ROW)*ROW+4-(action%ROW);

        if(act_board.node_type == AND){
            act_board.move(action, -1);
            // TODO: move to specific fields
        }
        if(action == 7 || action == 47){
            act_board.move(action, 1);
            if(is_left){
                act_board.move(11,-1);
            }
            else{
                act_board.node_type = OR;
                deactivate_line(act_board, 45);
            }
        }
        else if(action == 2 || action == 42){
            act_board.move(action, 1);
            if(is_left){
                act_board.node_type = OR;
                deactivate_line(act_board, 5);
            }
            else{
                act_board.move(37,-1);
            }
        }
        else if(action == 1 || action == 41 || action == 6 || action == 46 ||
                action == 3 || action == 43 || action == 8 || action == 48){
            act_board.move(action, 1);
            if(action == 6 || action == 8){
                act_board.move(action+5, -1);
                // TODO if not valid
            }
            else if(action == 46 || action == 48){
                deactivate_line(act_board, action==46?45:35);
            }
            else if(action == 1 || action == 41 || action == 3 || action == 43){
                act_board.move(center, -1);
                if(!is_left) act_board.white |= (1ULL << opposite);
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
    else{
        display(act_board, true, {action});
        assert(0);
    }
}
