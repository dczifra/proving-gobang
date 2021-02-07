#include "PNS.h"
#include "node.h"
#include "common_strategy.h"
#include "common.h"

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

Node* GeneralCommonStrategy::four_common_fields(Board& act_board, int action){
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    int& score = is_left ? act_board.score_left : act_board.score_right;

    if(act_board.node_type == AND){
        score = 1;
        Board next(act_board, action, -1); // Copy board, and move action with defender
        return add_or_create(next);
    }
    else{
        // Inner node
        Node* node = new InnerNode(2, OR);

        // Child 0: action defender, score = -1, OR
        score = -1;
        Board child0(act_board);
        child0.node_type = OR;
        child0.black |= (1ULL <<action);
        node->children[0] = add_or_create(child0);

        // Child 1
        // If attacker moves to inner-common ==> we can answer
        //                      outer.common ==> neighbour answers
        if((1ULL << action) & PNS::heuristic.forbidden_fields_inner){
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
        if(score == 1){
            board_int common_inner = PNS::heuristic.forbidden_fields_inner & side;
            board_int new_black = (1ULL << action) | act_board.black;
            if(__builtin_popcountll(new_black & common_inner) == 2) score = 1;
            else score = 2;
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
        if(score == 1){
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
            else score = -2;
            Board child0(act_board);
            child0.black |= (1ULL << action);
            node->children[0] = two_common_fields(child0, side, score);

            // === We cannot answer ===
            score = -1;
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
    if(score == 0){
        // two line
        return add_or_create(act_board);
    }
    else{
        act_board.forbidden_all &= (~side);
        if(score == 2) return add_or_create(act_board);     // Nothing to do
        else if(score == -2){ // Attacker moves to common
            act_board.white |= (side & ~(act_board.white | act_board.black));
            return add_or_create(act_board);
        }
        else{ // score == 1 or score == -1
            board_int free_common = side & ~(act_board.black | act_board.white);
            int first = __builtin_ctzl(free_common); // get first nonzero bit
            free_common &= ~(1ULL << first);
            int second = __builtin_ctzl(free_common);// get second nonzero bit
            //printf("First %d\n", first);
            //printf("Second %d\n", second);

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
                Node* node = new InnerNode(2, OR);

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
    int& score = is_left ? act_board.score_left : act_board.score_right;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    int num_common_fields = __builtin_popcountll(side & ~(b.white | b.black));

    if(num_common_fields == 4){
        return four_common_fields(act_board, action);
    }
    else if(num_common_fields == 3){
        return three_common_fields(act_board, action);
    }
    else if(num_common_fields == 2){
        // This is possible if and only if, the last two field is a 2-line
        act_board.white |= (1ULL << action);
        act_board.black |= ~(act_board.white | act_board.black)  & side;

        //std::cout<<"==========================\n";
        //display(act_board, true);
        return add_or_create(act_board);
    }
    else{
        display(act_board, true);
        std::cout<<num_common_fields<<std::endl;
        assert(0);
    }
}