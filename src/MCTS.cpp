#include "MCTS.h"

std::vector<int> MCTS::get_action_prob(Board& b, int curPlayer){
    std::vector<int> v;

    TreeNode* act_node = states[b]; // TODO b not in states
    for(int i=0;i<args.simulationNum;i++){
        search(act_node, curPlayer, -1);
    }

    return v;
}


float MCTS::search(TreeNode* node, int curPlayer, int last_action){
    // === If terminal node, return end-value ===
    int end_val = node->end;
    if(end_val!=0) return -end_val;

    // === Calculate UCT Score, and choose argmax ===
    int action = 0;
    float max = 0.0;
    for(int i=0;i<ACTION_SIZE;i++){
        float Ns_child = 1.0+((node->children[i]==nullptr)?0:node->children[i]->Ns);
        float UCT = args.cpuct*node->probs[i]*std::sqrt(node->Ns)/(Ns_child);
        float score = node->Q[i]+UCT;

        if(max<score){
            max=score;
            action = i;
        }
    }

    // === If node is leaf, extend ===
    if(node->children[action] == nullptr){
        // Extend Node
        node->children[action] = new TreeNode();
        TreeNode* child_node = node->children[action];

        child_node->board = Board(node->board, action, 1-curPlayer);
        child_node->end = child_node->board.get_winner(heuristic.compressed_lines_per_action[action]);

        Board temp_board(child_node->board);
        return curPlayer*play_random(temp_board, curPlayer);
    }
    else{
        float v = search(node->children[action], 1-curPlayer, action);
        // === Update Ns, Q, Nsa ===
        // Nsa was updated in search, we are going upwards
        int Nsa = node->children[action]->Ns;
        float Qsa = node->Q[action];
        node->Q[action]=((Nsa-1)*Qsa+v)/(Nsa);
        node->Ns+=1;

        return -v;
    }
}

int MCTS::play_random(Board& b, int player){
    int act;
    while(1){
        act = b.take_random_action(player);
        if(b.white_win(heuristic.compressed_lines_per_action[act])){
            return 0;
        }
        else if(b.black_win()){
            return 1;
        }
        player = 1-player;
    }
    std::cerr<<"Game not ended...\n";
    exit(1);
}

bool operator<(const Board& b1, const Board& b2){
    return b1.white<b2.white || (b1.white == b2.white && b1.black<b2.black);
}