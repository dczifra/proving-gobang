#include "MCTS.h"

std::vector<int> MCTS::get_action_prob(Board& b, int curPlayer){
    // Dont call for endGame positions ===
    // TODO handle situation

    std::vector<int> v(ACTION_SIZE);

    // === Get initial state ===
    TreeNode* act_node;
    if(states.find(b)==states.end()){
        extend_node(act_node, b, -1);
    }
    else{
        act_node = states[b];
    }

    // === Execute the simulations ===
    for(int i=0;i<args.simulationNum;i++){
        search(act_node, curPlayer, -1);
    }

    // === Collect the size for each action ===
    for(int i=0;i<ACTION_SIZE;i++){
        if(act_node->children[i]!=nullptr){
            v[i]=act_node->children[i]->Ns;
        }
        else{
            v[i]=0;
        }
    }

    return v;
}

float MCTS::search(TreeNode* node, int curPlayer, int last_action){
    // === If terminal node, return end-value ===
    //display(node->board, true);
    std::cout<<std::flush;
    int end_val = node->end;
    if(end_val!=0) return -end_val;

    // === Calculate UCT Score, and choose argmax ===
    int action = 0;
    float max = -1.0;
    for(int i=0;i<ACTION_SIZE;i++){
        if(!node->board.is_valid(i)) continue;

        float Ns_child = 1.0+((node->children[i]==nullptr)?0:node->children[i]->Ns);
        float UCT = args.cpuct*node->probs[i]*std::sqrt(node->Ns)/(Ns_child);
        float score = node->Q[i]+UCT;

        if(max<score){
            max=score;
            action = i;
        }
    }

    // === If node is leaf, extend ===
    float v;
    if(node->children[action] == nullptr){
        Board child_board(node->board, action, curPlayer);

        // === check wheather child_board already discoverd ===
        if(states.find(child_board)!=states.end()){
            node->children[action] = states[child_board];
            // === Go on with search ===
            v = search(node->children[action], -curPlayer, action);
        }
        else{
            // === Extend Node, and return random play result ===
            extend_node(node->children[action], child_board, action);

            Board temp_board(child_board);
            v = curPlayer*play_random(temp_board, curPlayer);
            node->children[action]->Ns+=1;
        }
    }
    else{
        v = search(node->children[action], -curPlayer, action);
    }
    
    // === Update Ns, Q, Nsa ===
    // Nsa was updated in search, we are going upwards
    int Nsa = node->children[action]->Ns;
    float Qsa = node->Q[action];
    node->Q[action]=((Nsa-1)*Qsa+v)/(Nsa);
    node->Ns+=1;

    return -v;
}

void MCTS::extend_node(TreeNode*& node, const Board& board, int last_action){
    node = new TreeNode();

    node->board = Board(board);
    if(last_action>=0)
        node->end = board.get_winner(heuristic.compressed_lines_per_action[last_action]);
    
    //node->probs = board.heuristic_mtx(heuristic.fields_on_compressed_lines);
    states[board]=node;
}

int MCTS::play_random(Board& b, int player){
    int act;
    while(1){
        act = b.take_random_action(player);
        if(b.white_win(heuristic.compressed_lines_per_action[act])){
            return 1;
        }
        else if(b.black_win()){
            return -1;
        }
        player = 1-player;
    }
    std::cerr<<"Game not ended...\n";
    exit(1);
}

bool operator<(const Board& b1, const Board& b2){
    return b1.white<b2.white || (b1.white == b2.white && b1.black<b2.black);
}