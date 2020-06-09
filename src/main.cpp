#define DEBUG false
#define TRANSPOSITION_TABLE false

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

void PNS_test(){
    Board b;
    //b.move(0,1);
    //b.move(4,-1);
    //b.move(19,1);

    PNS tree;
    PNSNode* node = new PNSNode(b, OR);
    
    for(int i=0;i<100000000;i++){
        tree.search(node);
        if(i%10000 == 0){
            std::cout<<"\r"<<node->pn<<" "<<node->dn<<std::flush;
        }

        if(node->pn*node->dn==0) break;
    }
    std::cout<<"                    \r"<<node->pn<<" "<<node->dn<<std::endl;
    play_with_tree(node, tree);
}

int main() {
    std::cout<<"Proving gobanggame..."<<std::endl;

    //MCTS_test();
    PNS_test();
    
    return 0;
}