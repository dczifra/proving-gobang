#define DEBUG false


#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "PNS.h"
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
    std::vector<int> moves = {0, ROW*COL/2-1, 3, ROW*COL/2-2, 9, 8, 15, ROW*COL/2+1, ROW*COL/2};
    //std::vector<int> moves = {0, ROW*COL/2-1, 8, ROW*COL/2-2, 9, 1, 15, ROW*COL/2+1, 23};
    Heuristic h;
    Board b;
    PNS tree;

    std::vector<int> parent(ACTION_SIZE, -1);
    std::vector<int> depth(ACTION_SIZE, -1);
    std::vector<int> low(ACTION_SIZE, -1);

    int player = 1;
    for(auto act: moves){
        b.move(act, player);
        tree.simplify_board(b, act, -1);

        display(b,true);
        player = -player;
    }
    //std::cout<<b.get_articulation_point(2, 0, parent, depth, low, h.linesinfo_per_field);
    
    Board::Artic_point p(2, &b, h.all_linesinfo.size(), h.linesinfo_per_field);
    auto comps = p.get_parts();
    std::cout<<"They are the components:\n";
    display(std::get<1>(comps), true);
    display(std::get<2>(comps), true);
    //auto mypair = p.get_articulation_point_bipartite(2, 0);
    //std::cout<<mypair.first<<" "<<mypair.second<<std::endl;

    print_v(p.depth);
    print_v(p.low);
}

void test_DFPN(){
    Board b;
    Heuristic h;
    int player = 1;
    //choose_problem(b,player);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, -1, -1, h);
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

int main() {
    std::cout<<"=== TEST ==="<<std::endl;

    //test_components();
    //test_DFPN();
    artic_point();

    return 0;
}