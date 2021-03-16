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
        if(action == 7 || action == 47 || action == 2 || action == 42){
            act_board.move(action, 1);
            act_board.move(action-1, -1);
            
            act_board.forbidden_all ^= (1ULL << action-1) | (1ULL << action) | (1ULL << action+1);
        }
        else if(action == 1 || action == 3 || action == 41 || action == 43 ||
                action == 6 || action == 8 || action == 46 || action == 48){
            int defender = (action / ROW)*ROW+2;

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
            //display(act_board, true);
            //display(act_board.forbidden_all, true);
            //act_board.forbidden_all &= ~side;
            // The last two remains a 2 line
            /*
            if(is_left){
                act_board.white |= (1ULL << 1) | (1ULL << 3); // also 2
                act_board.black |= (1ULL << 2);
            }
            else{
                act_board.white |= (1ULL << 46) | (1ULL << 48);      // also 47
                act_board.black |= (1ULL << 47);
            }
            act_board.forbidden_all &= ~side;
            */
        }
        else{
            assert(0);
        }
    }
    return add_or_create(act_board);
}


Node* GeneralCommonStrategy::four_common_fields(Board& act_board, int action){
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    int& score = is_left ? act_board.score_left : act_board.score_right;

    if(act_board.node_type == AND){
        //score = 1; // !!!!!
        score = 0; // No lunch, if there is no danger
        Board next(act_board, action, -1); // Copy board, and move action with defender
        return add_or_create(next);
    }
    else{
        // Inner node
        Node* node = new InnerNode(2, OR);

        // Child 0: action defender, score = -1, OR
        //score = -1;
        score = 0; // No lunch, if there is no danger
        Board child0(act_board);
        child0.node_type = OR; // not necessary
        child0.black |= (1ULL <<action);
        node->children[0] = add_or_create(child0);

        // Child 1
        // If attacker moves to inner-common ==> we can answer
        //                      outer.common ==> neighbour answers
        //if((1ULL << action) & PNS::heuristic.forbidden_fields_inner){
        if(is_left){
            // child1: action attacker, score = -1, AND
            score = -1;
            act_board.node_type = AND;
        }
        else{
            // child1: action attacker, score = 1, OR
            score = 1;
            act_board.node_type = OR;
        }
        Board child1(act_board);
        child1.white |= (1ULL << action);
        node->children[1] = add_or_create(child1);
        tree->update_node(node);
        return node;
    }
}

Node* GeneralCommonStrategy::three_common_fields(Board& act_board, int action){
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    int& score = is_left ? act_board.score_left : act_board.score_right;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    // If defender moves:
    if(act_board.node_type == AND){
        if(score == 0){
            score = 1;
        }
        else if(score == 1){
            board_int common_inner = PNS::heuristic.forbidden_fields_inner & side;
            board_int new_black = (1ULL << action) | act_board.black;
            if(__builtin_popcountll(new_black & common_inner) == 2) score = 1;
            //else score = 2;
            else score = 1;
        }
        else{
            assert(score == -1);
            score = -1;
        }
        Board next(act_board, action, -1);
        // TRAFO
        return two_common_fields(next, side, score);
    }
    // Attacker moves:
    else{
        if(score == 1 || score == 0){
            Node* node = new InnerNode(2, OR);
            // Child 0: Neighbour defender move, remains attacker move!!
            score = 1;
            Board child0(act_board);
            child0.black |= (1ULL << action);
            child0.node_type = OR;
            // TRAFO
            node->children[0] = two_common_fields(child0, side, score);

            // Child 1: Attacker move, our turn
            score = 0;
            Board child1(act_board);
            child1.white |= (1ULL << action);
            child1.node_type = AND;
            // TRAFO
            node->children[1] = two_common_fields(child1, side, score);
            tree->update_node(node);
            return node;
        }
        else{ // score == -1
            assert(score == -1);
            Node* node = new InnerNode(2, OR);
            // === if neighbour defender moves ===
            board_int common_outer = ~PNS::heuristic.forbidden_fields_inner & side;
            board_int new_black = (1ULL << action) | act_board.black;
            //std::cout<<"www\n";
            //display(new_black & common_outer, true);
            if(__builtin_popcountll(new_black & common_outer) == 2) score = -1;
            //else score = -2;
            else score = -1;
            Board child0(act_board);
            child0.black |= (1ULL << action);
            node->children[0] = two_common_fields(child0, side, score);

            // === We cannot answer ===
            score = 0;
            Board child1(act_board);
            child1.white |= (1ULL << action);
            child1.node_type = OR;
            // TRAFO
            node->children[1] = two_common_fields(child1, side, score);
            tree->update_node(node);
            return node;
        }
    }
}

Node* GeneralCommonStrategy::two_common_fields(Board& act_board, board_int side, int score){
    board_int free_common = side & ~(act_board.black | act_board.white);
    int first = __builtin_ctzl(free_common); // get first nonzero bit
    free_common &= ~(1ULL << first);
    int second = __builtin_ctzl(free_common);// get second nonzero bit
    //printf("First %d\n", first);
    //printf("Second %d\n", second);

    if(score == 0){
        // === Side-by-side ===
        if(second-first == 5){ // 2-line or split by side
            free_common = side & ~(act_board.black | act_board.white);
            act_board.white |= (free_common & ~PNS::heuristic.forbidden_fields_inner);
            act_board.forbidden_all &= (~side);
            return add_or_create(act_board);
        }
        // === Same side ===
        else if(second-first == 2){
            return add_or_create(act_board);
        }
        // === Diagonal ===
        else if(second-first == 3 || second-first == 7){
            free_common = side & ~(act_board.black | act_board.white);
            act_board.white |= (free_common & ~PNS::heuristic.forbidden_fields_inner);
            act_board.forbidden_all &= (~side);
            return add_or_create(act_board);
        }
        else{
            std::cout<<first<<" "<<second<<std::endl;
            display(act_board, true);
            assert(0);
        }
        // IF two emty is diagonal: 
        // two line
    }
    else{
        act_board.forbidden_all &= (~side);
        if(score == 2) return add_or_create(act_board);     // Nothing to do
        else if(score == -2){ // Attacker moves to common
            act_board.white |= (side & ~(act_board.white | act_board.black));
            return add_or_create(act_board);
        }
        else{ // score == 1 or score == -1
            if(score == 1){
                Node* node = new InnerNode(4, AND);
                // 4 child: XO OX XE EX  (E = empty, our )
                for(int i=0;i<4;i++){
                    Board child(act_board);
                    if(i%2 == 0){
                        child.black |= (1ULL << first);
                        if(i >= 2) child.white |= (1ULL << second);
                    }
                    else{
                        child.black |= (1ULL << second);
                        if(i >= 2) child.white |= (1ULL << first);
                    }
                    node->children[i] = add_or_create(child);
                }
                tree->update_node(node);
                return node;
            }
            else if(score == -1){
                Node* node = new InnerNode(2, AND);

                Board child0(act_board);
                child0.white |= (1ULL << first);
                node->children[0] = add_or_create(child0);

                Board child1(act_board);
                child1.white |= (1ULL << second);
                node->children[1] = add_or_create(child1);
                
                tree->update_node(node);
                return node;
            }
            else assert(0);
        }
    }
    assert(0);
}

Node* GeneralCommonStrategy::move_on_common(const Board& b, int action){
    Board act_board(b);
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    bool is_inner = (1ULL << action) & PNS::heuristic.forbidden_fields_inner;
    int& score = is_left ? act_board.score_left : act_board.score_right;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    int num_common_fields = __builtin_popcountll(act_board.forbidden_all & side & ~(b.white | b.black));

    //display(act_board, true, {action});
    //display(act_board.forbidden_all, true);
    //display(side & ~(b.white | b.black), true);
    //std::cout<<num_common_fields<<std::endl;
    if(num_common_fields == 6){
        return six_common_fields(act_board, action);
    }
    else if(num_common_fields == 2){
        if(act_board.node_type == AND){
            act_board.move(action, -1);
            // TODO: move to specific fields
        }
        else{
            //display(act_board, true, {action});
            //assert(act_board.node_type == OR);
            act_board.move(action, 1);
            int free = __builtin_ctzl(side & act_board.forbidden_all & ~(act_board.white | act_board.black));
            
            if(score == 1){
                if(!is_inner){
                    //act_board.move(free, -1)
                    int center = (action/ROW)*ROW+2;
                    int line = is_left?5:40;
                    line += (action>center?5:-5);
                    act_board.white &= ~(1ULL << line);
                    act_board.black |= (1ULL << line);
                }
                else{
                    //move free, but give up the other field
                    act_board.white |= (1ULL << free);
                }
            }
            else{
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
            }
        }
        act_board.forbidden_all &= ~side;
        return add_or_create(act_board);
    }
    else if(num_common_fields == 4 || num_common_fields == 3){
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
        else if(has(action, ~PNS::heuristic.forbidden_fields_inner & side)){
            act_board.move(action, 1);
            if(action == center){
                act_board.move(action+1, -1);
            }
            else{
                // Move free
            }
        }
        else if(has(action, PNS::heuristic.forbidden_fields_inner)){
            act_board.move(action, 1);
            if(action == center){
                act_board.move(action+1, -1);
            }
            else{
                if(!is_left){
                    act_board.node_type = OR;
                    act_board.white &= ~(1ULL << 40);
                    act_board.black |= (1ULL << 40);
                }
                else{
                    act_board.node_type = OR;
                    act_board.white &= ~(1ULL << 5);
                    act_board.black |= (1ULL << 5);
                }
            } 
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
/*
    if(num_common_fields == 4){
        return four_common_fields(act_board, action);
    }
    else if(num_common_fields == 3){
        return three_common_fields(act_board, action);
    }
    else if(num_common_fields == 2){
        // This is possible if and only if, the last two field is a 2-line
        if(act_board.node_type == OR){
            act_board.white |= (1ULL << action);
            act_board.black |= ~(act_board.white | act_board.black)  & side;
        }
        else{
            act_board.black |= (1ULL << action);
            act_board.white |= ~(act_board.white | act_board.black)  & side;
        }
        //std::cout<<"==========================\n";
        //display(act_board, true);
        return add_or_create(act_board);
    }
    else{
        display(act_board, true);
        std::cout<<num_common_fields<<std::endl;
        assert(0);
    }
*/
}
