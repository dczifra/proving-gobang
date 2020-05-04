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
    //for(auto lines: heuristic.lines_per_action[0]){
    //    for(auto field: lines){
    //        std::cout<<field.first<<" "<<field.second<<std::endl;
    //    }
    //}
    //for(auto line: heuristic.compressed_lines_per_action[32]){
    //    display(line, true);
    //}

    Board b;
    int player = 0;

    while(1){
        int act = b.take_random_action(player);
        std::cout<<"Action: "<<act<<std::endl;
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