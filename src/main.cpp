#define DEBUG false
#define TRANSPOSITION_TABLE false

#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "testboard.h"
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
    b.move(1,1);
    b.move(4,-1);
    b.move(9,1);
    PNS tree;
    PNSNode* node = new PNSNode(b, OR);
//tree.search(node);
    //std::cout<<node->pn<<" "<<node->dn<<std::endl;
    
    for(int i=0;i<10000000;i++){
        tree.search(node);
        //std::cout<<"=================="<<i<<"===========\n";
        if(i%10000 == 0) std::cout<<node->pn<<" "<<node->dn<<std::endl;
        if(node->pn*node->dn==0) break;
    }
    std::cout<<node->pn<<" "<<node->dn<<std::endl;
}

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

int main() {
    std::cout<<"Proving gobanggame..."<<std::endl;

    //MCTS_test();
    //human_play();
    PNS_test();
    
    return 0;
}