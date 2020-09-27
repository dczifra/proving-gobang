#include "play.h"
#include "artic_point.h"

#include<sstream>
#include<fstream>

Play::Play(std::string filename, bool disproof){
    player = 1;
    choose_problem(board, player, disproof);
    human_player = -player;

    // === Read Solution Tree ===
    read_solution(filename);
    printf("Proof/disproof tree size: %zu\n", tree.states.size());

    build_tree();
}

NodeType Play::choose_problem(Board& board, int& player, bool disproof){
    if(disproof) board.move({0,1}, player);
    //board.move({0,1,23}, player);

    return (player==1?OR:AND);
}

void Play::read_solution(std::string filename){
    std::ifstream file(filename.c_str());
    if(file.is_open()){
        std::cout<<"Processing file...\n";
    }
    else{
        std::cout<<"File not found: "<<filename<<std::endl;
    }

    std::string s;
    while(std::getline (file, s)){
        //std::cout<<s<<std::endl;
        unsigned int pn, dn;
        Board b;

        std::stringstream sstream(s);
        sstream>>b.white>>b.black>>b.node_type>>pn>>dn;
        if(tree.states.find(b) == tree.states.end()){
            PNS::PNSNode* node = new PNS::PNSNode(b, -1, -1, -1,tree.heuristic);
            node->pn = pn;
            node->dn = dn;
            tree.states[b] = node;
        }
        else{
            //printf("Duplicated state:\n");
            //display(b, true);
        }
    }
}

int Play::move_human(){
    int row, col, act = -1;
    std::cin>>row>>col;
    act = col*ROW+row;

    while(!board.is_valid(act)){
        printf("Not valid action: %d\n", act);
        std::cin>>row>>col;
        act = col*ROW+row;
    }
    
    board.move(act, human_player);
    tree.simplify_board(board, act, -1);

    return act;
}

Board Play::move_in_solution(int last_act, int& act, std::vector<int>& color){
    // === Simplification and one way ===
    Board next(board, last_act, player);
    tree.simplify_board(next, last_act, -1);
    color = {last_act};

    // One-way steps
    while(!tree.game_ended(next, last_act)){
        int temp_act = next.one_way(tree.get_all_lines());
        if(temp_act > -1){
            last_act = temp_act;
            next.move(last_act, next.node_type== OR ? 1 : -1);
            color.push_back(last_act);
        }
        else break;
    }
    act = last_act;

    // components
    Artic_point comps(next, tree.heuristic.all_linesinfo, tree.heuristic.linesinfo_per_field);
    int artic_point;
    Board small_board, big_board;
    std::tie(artic_point, small_board, big_board) = comps.get_parts();

    if(artic_point > -1){
        //printf("=== ARTIC POINT ===\n");
        Board small_board_f(small_board);
        small_board_f.flip();
        Board big_board_f(big_board);
        big_board_f.flip();

        if(tree.states.find(small_board) != tree.states.end() || tree.states.find(small_board_f) != tree.states.end()){
            printf("Small comp win\n");
            display(small_board, true);
            return small_board;
        }
        else if(tree.states.find(big_board) != tree.states.end() || tree.states.find(big_board_f) != tree.states.end()){
            printf("Big comp win\n");
            display(big_board, true);
            return big_board;
        }
        else{
            big_board.white |= ((1ULL)<<artic_point);
            big_board_f = big_board;
            big_board_f.flip();
            if(tree.states.find(big_board) != tree.states.end() || tree.states.find(big_board_f) != tree.states.end()){
                printf("Small-Big combo win win\n");
                display(big_board, true);
                return big_board;
            }
        }
    }

    return next;
    //display(next, true);
}

void Play::play_with_solution(){
    int act;

    while(!tree.game_ended(board, act)){
        std::vector<int> color;
        act = -1;
        // === Human player can choose ===
        if(player == human_player ){
            act = move_human();
        }
        else{
            // === Find the next child in Solution Tree ===
            int curr_action=0;
            for(;curr_action<ACTION_SIZE;curr_action++){
                if(!board.is_valid(curr_action)) continue;

                Board next = move_in_solution(curr_action, act, color);

                Board reversed(next);
                reversed.flip();
                // === If found the child, move current action ===
                if(tree.states.find(next)!=tree.states.end() || tree.states.find(reversed)!=tree.states.end()){
                    board = next;
                    break;
                }
            }
            if(curr_action== ACTION_SIZE){
                printf("Not found next step\n");
                PNS::PNSNode* node = new PNS::PNSNode(board, -1, -1, -1, tree.heuristic);
                PNS new_tree;
                new_tree.evalueate_node_with_PNS(node, false, true);
                printf("Node pn: %d States size: %d\n", node->pn, (int) new_tree.states.size());
                break;
            }
        }
        color.push_back(act);

        player = get_player(board.node_type);
        printf("Action: %d\n", act);
        display(board, true, color);
    }
    std::cout<<"END\n";
}

void Play::build_tree(){
    PNS new_tree;
    for(auto state: tree.states){
        for(int i=0;i<ACTION_SIZE;i++){
            Board next = new_tree.extend(state.second, i, false);
            if(tree.states.find(next) != tree.states.end()){
                state.second->children[i] = tree.states[next];
            }
        }
    }
}

void Play::play_with_solution2(){
    int act;

    while(!tree.game_ended(board, act)){
        std::vector<int> color;
        act = -1;
        // === Human player can choose ===
        if(player == human_player ){
            act = move_human();
        }
        else{
            // === Find the next child in Solution Tree ===
            if(board.node_type == OR) act = tree.get_min_children(tree.states[board], PN, true);
            else act = tree.get_min_children(tree.states[board], DN, true);

            int curr_action=act;
            if(curr_action == ACTION_SIZE or curr_action == -1){
                printf("Not found next step\n");
                PNS::PNSNode* node = new PNS::PNSNode(board, -1, -1, -1, tree.heuristic);
                PNS new_tree;
                new_tree.evalueate_node_with_PNS(node, false, true);
                printf("Node pn: %d States size: %d\n", node->pn, (int) new_tree.states.size());
            }
            else
            {
                act = curr_action;
                //Board next = move_in_solution(curr_action, act, color);
                board = tree.states[board]->children[act]->board;
                display(board, true);
                std::cout<<board.node_type<<std::endl;
            }

        }
        color.push_back(act);

        player = get_player(board.node_type);
        printf("Action: %d\n", act);
        display(board, true, color);
    }
    std::cout<<"END\n";
}