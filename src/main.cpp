#define DEBUG false


#include "common.h"
#include "heuristic.h"
#include "board.h"
//#include "gobanggame.h"
#include "MCTS.h"

int play(Board& b, int player, const Heuristic& heuristic){
    int act;
    while(1){
        act = b.take_random_action(player);
        if(b.white_win(heuristic.compressed_lines_per_action[act])){
#if DEBUG
            std::cout<<"\nWhite win";
#endif
            return 0;
        }
        else if(b.black_win()){
#if DEBUG
            std::cout<<"\nBlack win";
#endif
            return 1;
        }
        player = 1-player;
        //display(b, false);
    }
}

int main() {
    std::cout<<"Proving gobanggame..."<<std::endl;

    Heuristic heuristic;
    auto lines = heuristic.fields_on_compressed_lines;
    
    Board b;

    for(int i=0;i<500000;i++){
        b.init();
        int player = 0;
        int win = play(b,player,heuristic);
        //display(b, true);
    }

    b.init();
    auto mtx = b.heuristic_mtx(lines);
    print_mtx(mtx);
    return 0;
}