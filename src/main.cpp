#include "common.h"
#include "heuristic.h"
#include "board.h"
//#include "gobanggame.h"
#include "MCTS.h"

int main() {
    std::cout<<"Proving gobanggame..."<<std::endl;

    Heuristic heuristic;
    auto lines = heuristic.fields_on_compressed_lines;

    //GobangGame game();

    Board b;
    int player = 0;
    while(1){
        int act = b.take_random_action(player);
        display(b, true);
        if(b.white_win(heuristic.compressed_lines_per_action[act])){
            std::cout<<"White win\n";
            break;
        }
        else if(b.black_win()){
            std::cout<<"Black win\n";
            break;
        }

        player = 1-player;
    }

    //auto mtx = b.heuristic_mtx(lines);
    //std::cout<<"www\n";
    //print_mtx(mtx);

    return 0;
}