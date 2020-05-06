#define DEBUG false


#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "MCTS.h"

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

    for(int i=0;i<500000;i++){
        b.init();
        int player = 1;
        int win = play(b,player,heuristic);
        //display(b, true);
    }

    b.init();
    std::array<float, ACTION_SIZE> mtx = b.heuristic_mtx(lines);
    print_mtx(mtx);
}

int main() {
    std::cout<<"Proving gobanggame..."<<std::endl;

    Board board;
    //board.move(0, 1);
    //board.move(1, -1);
    //std::cout<<board.is_valid(0)<<std::endl;
    //std::cout<<board.is_valid(1)<<std::endl;
    //std::cout<<board.is_valid(3)<<std::endl;
    //exit(1);

    MCTS mcts;
    std::vector<int> probs = mcts.get_action_prob(board, 1);
    print_mtx(probs);

    return 0;
}