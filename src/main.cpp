#define DEBUG false
#define TRANSPOSITION_TABLE false

#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "MCTS.h"
#include "PNS.h"

int play(Board& b, int player, const Heuristic& heuristic){
    int act;
    while(1){
        act = b.take_random_action(player);
        if(b.white_win(heuristic.compressed_lines_per_action[act])){
#if DEBUG
            std::cout<<"\nWhite win";
#endif
            return 1;
        }
        else if(b.black_win()){
#if DEBUG
            std::cout<<"\nBlack win";
#endif
            return -1;
        }
        player = -player;
        //display(b, false);
    }
}

void random_playes(){
    Heuristic heuristic;
    auto lines = heuristic.fields_on_compressed_lines;
    
    Board b;

    for(int i=0;i<100000;i++){
        b.init();
        int player = 1;
        int win = play(b,player,heuristic);
        //display(b, true);
    }

    b.init();
    std::array<float, ACTION_SIZE> mtx = b.heuristic_mtx(lines);
    print_mtx(mtx);
}

void MCTS_test(){
    Board board;
    MCTS mcts;
    std::vector<int> probs = mcts.get_action_prob(board, 1);
    print_mtx(probs);
}

void PNS_test(){
    Board b;

    PNS tree;
    PNSNode* node = new PNSNode(b);
    tree.search(node);
}

int main() {
    std::cout<<"Proving gobanggame..."<<std::endl;

    MCTS_test();
    
    return 0;
}