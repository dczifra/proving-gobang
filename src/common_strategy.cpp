#include "PNS.h"
#include "node.h"
#include "common_strategy.h"

Node* GeneralCommonStrategy::four_common_fields(Board& act_board, int action, Args* args){
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    int& score = is_left ? act_board.score_left : act_board.score_right;

    if(act_board.node_type == AND){
        score = 1;
        Board next(act_board, action, -1);
        return new PNSNode(next, args);
    }
    else{
        // Inner node
        Node* node = new InnerNode(2, OR);

        // Child 0: action defender, score = -1, OR
        score = -1;
        Board child0(act_board);
        child0.node_type = OR;
        child0.black |= (1ULL <<action);
        node->children[0] = new PNSNode(child0, args);

        // Child 1
        // If attacker moves to inner-common ==> we can answer
        //                      outer.common ==> neighbour answers
        if((1ULL << action) & PNS::heuristic.forbidden_fields_inner){
            // child1: action attacker, score = -1, AND
            score = -1;
            Board child1(act_board);
            child1.node_type = AND;
            child1.white |= (1ULL << action);
            node->children[1] = new PNSNode(child1, args);
        }
        else{
            // child1: action attacker, score = 1, OR
            score = 1;
            Board child1(act_board);
            child1.node_type = OR;
            child1.white |= (1ULL << action);
            node->children[1] = new PNSNode(child1, args);
        }
        return node;
    }
}

Node* GeneralCommonStrategy::three_common_fields(Board& act_board, int action, Args* args){
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    int& score = is_left ? act_board.score_left : act_board.score_right;
    board_int side = is_left ? PNS::heuristic.forbidden_fields_left : PNS::heuristic.forbidden_fields_right;
    // If defender moves:
    if(act_board.node_type == AND){
        if(score == 1){
            score = 2;
        }
        else{
            assert(score == -1);
            score = 1;
        }
        Board next(act_board, action, -1);
        // TRAFO
        return new PNSNode(next, args);
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
            node->children[0] = new PNSNode(child0, args);

            // Child 1: Attacker move, our turn
            score = 0;
            Board child1(act_board);
            child1.white |= (1ULL << action);
            child1.node_type = AND;
            // TRAFO
            node->children[1] = new PNSNode(child1, args);
            return node;
        }
        else{
            assert(score == -1);
            // === We cannot answer ===
            score = -1;
            Board next(act_board);
            next.white |= (1ULL << action);
            next.node_type = OR;
            // TRAFO
            return new PNSNode(next, args);
        } 
    }
}

Node* GeneralCommonStrategy::two_common_fields(Board& act_board, board_int side, int score, Args* args){
    if(score == 0){

    }
    else{
        act_board.forbidden_all ^= side;
        if(score == 2) 0;     // Nothing to do
        else if(score == -2){ // Attacker moves to common
            act_board.white |= (side & ~(act_board.white | act_board.black));
        }
        else if(score == 1){
            // ...
        }
        else if(score == -1){
            // ...
        }
        else assert(0);
    }

}

Node* GeneralCommonStrategy::move_on_common(const Board& b, int action, Args* args){
    Board act_board(b);
    bool is_left = (1ULL << action) & PNS::heuristic.forbidden_fields_left;
    int& score = is_left ? act_board.score_left : act_board.score_right;
    int num_common_fields = __builtin_popcountll(PNS::heuristic.forbidden_all & (b.white | b.black));

    if(num_common_fields == 4){
        return four_common_fields(act_board, action, args);
    }
    else if(num_common_fields == 3){
        return three_common_fields(act_board, action, args);
    }
    else if(num_common_fields == 2){
        // This is possible if and only if, the last two field is a 2-line
        act_board.white |= (1ULL << action);
        if(is_left) act_board.black |= (act_board.white | act_board.black)  & PNS::heuristic.forbidden_fields_left;
        else act_board.black |= (act_board.white | act_board.black)  & PNS::heuristic.forbidden_fields_right;
    }
    else assert(0);
}