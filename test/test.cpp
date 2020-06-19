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

/**
 * Description:
 *     2 random player's play, repeated 1000 times
 * */
void random_playes(const Board& basic, int times = 1000){
    Heuristic heuristic;
    auto lines = heuristic.fields_on_compressed_lines;
    
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
    auto lines = heuristic.fields_on_compressed_lines;
    
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
        if(b.white_win(heuristic.compressed_lines_per_action[act])){
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
    b.move(6, 1);
    b.move(9, -1);

    auto val = b.get_free_fields_graph(h.fields_on_compressed_lines);
}

int main() {
    std::cout<<"=== TEST ==="<<std::endl;

    test_components();

    return 0;
}