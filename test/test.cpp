#define DEBUG false


#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "PNS.h"
#include "artic_point.h"
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
    int player = 1;
    //std::vector<int> moves = {0, 12, 1, 13, 2, 14, 5, 15, 6, 3, 7, 6, 10, 9};
    std::vector<int> moves = {12, 13, 14, 29, 26, 5, 6, 4, 15, 7, 0, 2};
    for(auto act: moves){
        b.move(act, player);
        player = -player;
        b.remove_lines_with_two_ondegree(h.all_linesinfo);
        b.remove_2lines_all(h.all_linesinfo);
        b.remove_dead_fields(h.linesinfo_per_field, act);
    }
    display(b,true);
}

void test_components2(){
    Heuristic h;
    Board b;
    int player = 1;

    std::vector<int> moves = {1, 4, 2, 5};

    for(auto act: moves){
        b.move(act, player);
        if(player == -1){
            b.remove_dead_fields(h.linesinfo_per_field, act);
            //b.remove_small_components(h.all_linesinfo);
            //b.remove_2lines_all(h.all_linesinfo);
            b.remove_2lines(h.linesinfo_per_field, act);
        }
        display(b,true);
        player = -player;
    }
}

void artic_point(){
    std::vector<int> moves = {0,1,8,10, 29, 24, 25, 30, 5, 2, 17, 21};

    Board b;
    PNS tree;

    int player = 1;
    for(auto act: moves){
        b.move(act, player);
        //tree.simplify_board(b);
        //display(b,true);
        player = -player;
    }
    display(b, true);

    //PNS::PNSNode* node= new PNS::PNSNode(b, -1, tree.heuristic);
    //b = tree.extend(node, 21, false);
    //display(b, true);
    tree.evaluate_components(b, -1);
    
    //Artic_point p(b, tree.heuristic.all_linesinfo, tree.heuristic.linesinfo_per_field);
    //auto comps = p.get_parts();
    //printf("They are the components: %d \n", std::get<0>(comps));
    //display(std::get<1>(comps), true);
    //display(std::get<2>(comps), true);
}

void test_DFPN(){
    Board b;
    Heuristic h;
    int player = 1;
    //choose_problem(b,player);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, h);
    tree.init_DFPN_search(node);
    
    unsigned int i = 0;
    while(1){
        tree.DFPN_search(node);
        if(i%10000 == 0){
            tree.stats(node);
        }
        if(node->pn*node->dn==0) break;
        i++;
    }
    tree.stats(node);
}

void canonical_order2(){
    Board b1;
    b1. white = 528401ULL;
    b1.black = 131334ULL;
    b1.move(14, 1);
    b1.move(9, -1);
    Board b2;
    b2.white = 528409ULL;
    b2.black = 131846ULL;

    display(b1, true);
    display(b2, true);

    PNS tree;
    tree.isom_machine.get_conversion(b1, b2, tree.heuristic.all_linesinfo);
}

void canonical_order(){
    PNS tree;
    Board b1;
    int player = 1;
    std::vector<int> moves = {1,4,8,9, 13, 14, 18, 12, 17, 6, 0, 16, 11, 2, 19, 10};
    for(auto m: moves){b1.move(m, player);player=-player;}
    tree.simplify_board(b1);

    Board b2;
    moves = {0,1,2,3,4,14, 19, 15, 5, 18, 6, 17, 8, 9, 10, 11};
    for(auto m: moves){b2.move(m, player);player=-player;}
    tree.simplify_board(b2);
    b2.black |= ((1ULL)<<12);

    display(b1, true);
    display(b2, true);

    tree.isom_machine.get_conversion(b1, b2, tree.heuristic.all_linesinfo);
}

Heuristic PNS::heuristic;
CanonicalOrder PNS::isom_machine;
int main() {
    std::cout<<"### TEST ###"<<std::endl;

    //test_components();
    //test_DFPN();
    //artic_point();
    canonical_order();
    //canonical_order();
    //PNS::PNSNode* node = new PNS::PNSNode(b, -1, tree.heuristic);
    //tree.evalueate_node_with_PNS(node, true, false);
    //tree.stats(node);

    return 0;
}
