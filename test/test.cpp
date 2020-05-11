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

void random_playes(const Board& basic){
    Heuristic heuristic;
    auto lines = heuristic.fields_on_compressed_lines;
    
    int sum =0;
    for(int i=0;i<1000;i++){
        Board b(basic);
        int player = 1;
        sum += (1+play(b,player,heuristic))/2;
        //display(b, true);
    }
    std::cout<<sum<<std::endl;

}

int main() {
    std::cout<<"=== TEST ==="<<std::endl;
    Board b;
    b.move(1,-1);
    b.move(45,-1);
    random_playes(b);

    return 0;
}