#define DEBUG false
#define TRANSPOSITION_TABLE false
#define RECURSIVE_LINE_SEARCH true
// this macro does not work


#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "PNS.h"

struct Args{
    bool log = false;
    bool play = false;

    Args(int argc, char* argv[]){
        int i=0;
        while(i<argc){
            if((std::string) argv[i] == "--log") log = true;
            else if((std::string) argv[i] == "--play") play = true;
            else if((std::string )argv[i] == "--help"){
                std::cout<<"Help for AMOBA\nARGS:\n";
                std::cout<<"--play: Play with tree\n";
                std::cout<<"--log: log root PN and DN\n";
            }
            i++;
        }
    }
};

void play_with_tree(PNS::PNSNode* node, const PNS& tree){
    Heuristic heuristic;

    int player = get_player(node->type);
    const int human_player = (node->pn == 0?-1:1);
    PNS::PNSNode* act_node = node;
    int act = -1;

    while(1){
        if(player == human_player ){
            int act0 = act_node->board.one_way(tree.get_all_lines());
            if(act0 >= 0) std::cout<<act0<<" is a must\n";
            
            std::cin>>act;
        }
        else{
            if(player == 1) act = tree.get_min_children(act_node, PN, true);
            else act = tree.get_min_children(act_node, DN, true);
        }

        act_node = act_node->children[act];

        // === Check game over ===
        if((act_node->type == AND) && act_node->board.white_win(tree.get_lines(act))){
            printf("(1) Game over (Winner: %s)\n", (player==1)?"white":"black");
            break;
        }
        else if((act_node->type == OR) && act_node->board.heuristic_stop(tree.get_all_lines())){
            printf("(2) Game over (Winner: %s)\n", (player==1)?"white":"black");
            break;
        }
        //player = -player;
        player = get_player(act_node->type);
        std::cout<<"Action:\n";
        display(act_node->board, true);

    }
    display(act_node->board, true);
}

NodeType choose_problem(Board& b, int& player){
    //b.move({0,1,ROW*COL -1}, player);
    //b.move({0,1,23}, player);

    return (player==1?OR:AND);
}

void PNS_test(Args& args){
    Board b;
    int player = 1;
    choose_problem(b,player);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, -1, -1, tree.heuristic);
    tree.init_PN_search(node);

    unsigned int i = 0;
    while(1){
        tree.PN_search(node);
        if(i%10000 == 0 && args.log){
            tree.stats(node);
        }
        if(node->pn*node->dn==0) break;
        i++;
    }
    tree.stats(node);
    
    std::string filename("../data/"+std::to_string(ROW)+"x"+std::to_string(COL)+".csv");
    std::ofstream logfile(filename);
    tree.log_solution_min(node, logfile);
    if(args.play) play_with_tree(node, tree);
}

void DFPNS_test(Args& args){
    Board b;
    int player = 1;
    //choose_problem(b,player);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, -1, -1, tree.heuristic);
    tree.init_DFPN_search(node);
    
    unsigned int i = 0;
    while(1){
        tree.DFPN_search(node);
        if(i%10000 == 0 && args.log){
            tree.stats(node);
        }
        if(node->pn*node->dn==0) break;
        i++;
    }
    tree.stats(node);
}

int main(int argc, char* argv[]) {
    std::cout<<"Proving gobanggame..."<<std::endl;

    Args args(argc, argv);
    //DFPNS_test(argc>1);
    PNS_test(args);

    return 0;
}
