#define DEBUG false
#define TRANSPOSITION_TABLE false

#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "MCTS.h"
#include "PNS.h"


void MCTS_test(){
    Board board;
    MCTS mcts;
    std::vector<int> probs = mcts.get_action_prob(board, 1);
    print_mtx(probs);
}

void play_with_tree(PNSNode* node, PNS tree){
    int player = get_player(node->type);
    const int human_player = -player;
    PNSNode* act_node = node;
    int act = -1;
    while(1){
        if(player == human_player ){
            std::cin>>act;
        }
        else{
            if(player == 1) act = tree.get_min_children(act_node, PN, true);
            else act = tree.get_min_children(act_node, DN, true);
        }

        act_node = act_node->children[act];

        // === Check game over ===
        if(act_node->board.get_winner(tree.get_lines(act))){
            printf("Game over (Winner: %s)\n", (player==1)?"white":"black");
            break;
        }

        player = -player;
        display(act_node->board, false);
    }
    display(act_node->board, true);
}

NodeType choose_problem(Board& b, int& player){
    //b.move({0,1,27}, player);
    //b.move({0,1,23}, player);

    return (player==1?OR:AND);
}

void PNS_test(){
    Board b;
    int player = 1;
    
    NodeType starter = choose_problem(b,player);

    PNS tree;
    PNSNode* node = new PNSNode(b, starter);
    tree.add_state(b,node);
    
    unsigned int i = 0;
    while(1){
        tree.search(node);
        if(i%10000 == 0){
            std::cout<<"\r"<<node->pn<<" "<<node->dn<<std::flush;
        }
        if(node->pn*node->dn==0) break;
        i++;
    }
    std::cout<<"                    \r"<<node->pn<<" "<<node->dn<<std::endl;
    
    std::string filename("../data/"+std::to_string(ROW)+"x"+std::to_string(COL)+".csv");
    std::ofstream logfile(filename);
    tree.log_solution_min(node, logfile);
    //play_with_tree(node, tree);
}

int main() {
    std::cout<<"Proving gobanggame..."<<std::endl;

    //MCTS_test();
    PNS_test();
    Board b;
    b.move(1,1);
    //b.flip();
    display(b, true);
    
    return 0;
}