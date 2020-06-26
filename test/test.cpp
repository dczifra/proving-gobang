#define DEBUG false


#include "common.h"
#include "heuristic.h"
#include "board.h"

// =================================================================
//                     TEST THE BOARD's GOODNESS
// =================================================================
int play(Board& b, int player, const Heuristic& heuristic){
    int act;
    while(1){
        act = b.take_random_action(player);
        if(b.white_win(heuristic.linesinfo_per_field[act])){
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

/**
 * Description:
 *     2 random player's play, repeated 1000 times
 * */
void random_playes(const Board& basic, int times = 1000){
    Heuristic heuristic;
    auto lines = heuristic.all_linesinfo;
    
    int sum =0;
    for(int i=0;i<times;i++){
        Board b(basic);
        int player = 1;
        sum += (1+play(b,player,heuristic))/2;
        //display(b, true);
    }
    std::cout<<sum<<std::endl;

}

/**
 * Description:
 *     Human player vs random player 
 * */
void human_play(){
    Heuristic heuristic;
    auto lines = heuristic.all_linesinfo;
    
    Board b;
    int act;
    int player = 1;
    while(1){
        if(player == 1){
            std::cin>>act;
            b.move(act, player);
        }
        else{
            act = b.take_random_action(player);
        }
        if(b.white_win(heuristic.linesinfo_per_field[act])){
            printf("White win\n");
            break;
        }
        else if(b.black_win()){
            printf("Black win\n");
            break;
        }
        player = -player;
        display(b, false);
    }
    display(b, true);
}

// =================================================================
//                     TEST THE BOARD's GOODNESS
// =================================================================
void test_components(){
    Heuristic h;

    Board b;
    b.move(12, -1);
    b.move(13, -1);
    b.move(14, -1);
    b.move(15, -1);
    b.move(3, -1);
    b.move(6, -1);
    b.move(9, -1);

    b.move(1, 1);
    b.move(21, 1);

    b.remove_small_components(h.all_linesinfo);
    display(b,true);
}

void test_components2(){
    Heuristic h;

    //std::vector<int> moves = {8, 20, 13, 21, 18, 22, 25, 23};
    //std::vector<int> moves = {1,4,25,20, 11, 6, 13, 5, 22, 19, 16, 21, 10, 7};
    std::vector<int> moves = {1, 4, 2, 5, 6, 3};

    Board b;
    int player = 1;

    for(auto act: moves){
        b.move(act, player);
        //b.remove_small_components(h.all_linesinfo);
        if(player == -1) b.remove_dead_fields(h.linesinfo_per_field, act);
        display(b,true);
        player = -player;
    }
}

int main() {
    std::cout<<"=== TEST ==="<<std::endl;

    test_components2();

    return 0;
}