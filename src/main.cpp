#define DEBUG false
#define TRANSPOSITION_TABLE false
#define RECURSIVE_LINE_SEARCH true
// this macro does not work


#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "PNS.h"

struct Args{
    bool log = false;
    bool play = false;
    bool test = false;

    Args(int argc, char* argv[]){
        int i=0;
        while(i<argc){
            if((std::string) argv[i] == "--log") log = true;
            else if((std::string) argv[i] == "--play") play = true;
            else if((std::string) argv[i] == "--test") test = true;
            else if((std::string )argv[i] == "--help"){
                std::cout<<"Help for AMOBA\nARGS:\n";
                std::cout<<"--play: Play with tree\n";
                std::cout<<"--log: log root PN and DN\n";
                std::cout<<"--test: Tets mode, you can play the solution\n";
            }
            i++;
        }
    }
};

int get_game_ended(Board& b, int action, Heuristic& h){
    if((b.node_type == AND) && action > -1 && b.white_win(h.linesinfo_per_field[action])){
        return 1;
    }
    else if((b.node_type == OR) && b.heuristic_stop(h.all_linesinfo)){
        return -1;
    }
    else return 0;
}

NodeType choose_problem(Board& b, int& player){
    //b.move({0,1,ROW*COL -1}, player);
    //b.move({0,1,23}, player);

    return (player==1?OR:AND);
}

void read_solution(std::string filename, std::unordered_map<Board, PNS::PNSNode*, Board_Hash>& states){
    std::ifstream file(filename);
    if(file.is_open()){
        std::cout<<"Processing file...\n";
    }
    else{
        std::cout<<"File not found: "<<filename<<std::endl;
    }

    std::string s;
    std::getline (file, s);
    while(s != "" && s != "\n"){
        //std::cout<<s<<std::endl;
        unsigned int pn, dn;
        Board b;
        std::stringstream sstream(s);
        sstream>>b.white>>b.black>>b.node_type>>pn>>dn;
        states[b] = nullptr;

        std::getline (file, s);
    }
}

void play_with_solution(std::string filename){
    // === Read Solution Tree ===
    std::unordered_map<Board, PNS::PNSNode*, Board_Hash> states;
    read_solution(filename, states);
    printf("Proof/disproof tree size: %zu\n", states.size());

    // === Init variables ===
    PNS tree;
    Board b;
    int act=-1, human_player, player = 1;
    choose_problem(b,player);
    human_player = -player;


    while(get_game_ended(b, act, tree.heuristic) == 0){
        std::vector<int> color;
        // === Human player can choose ===
        act = b.one_way(tree.heuristic.all_linesinfo);
        if(act>=0){
            b.move(act, player);
            std::cout<<"One-way end detected"<<std::endl;
            //tree.simplify_board(b, act, -1);
        }
        else if(player == human_player ){
            std::cin>>act;
            b.move(act, human_player);
            tree.simplify_board(b, act, -1);
        }
        else{
            // === Find the next child in Solution Tree ===
            for(int i=0;i<ACTION_SIZE;i++){
                if(!b.is_valid(i)) continue;

                color.resize(0);

                // === Simplification and one way ===
                Board next(b, i, player);
                tree.simplify_board(next, i, -1);
                int last_act = i;
                color = {i};
                while(!tree.game_ended(next, last_act)){
                    int temp_act = next.one_way(tree.get_all_lines());
                    if(temp_act > -1){
                        last_act = temp_act;
                        next.move(last_act, next.node_type== OR ? 1 : -1);
                        color.push_back(last_act);
                    }
                    else break;
                }
                //display(next, true);

                Board reversed(next);
                reversed.flip();
                // === We found the child ===
                if(states.find(next)!=states.end() || states.find(reversed)!=states.end()){
                    b = next;
                    act = last_act;
                    break;
                }
            }
        }
        color.push_back(act);

        player = get_player(b.node_type);
        printf("Action: %d\n", act);
        display(b, true, color);
    }
    std::cout<<"END\n";
}

void play_with_tree(PNS::PNSNode* node, const PNS& tree){
    PNS t;
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
        display(act_node->board, true, {act});
    }
    display(act_node->board, true);
}

void PNS_test(Args& args){
    Board b;
    int player = 1;
    choose_problem(b,player);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, -1, -1, tree.heuristic);
    tree.init_PN_search(node);

    tree.evalueate_node_with_PNS(node, args.log);
    tree.stats(node, true);
    
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

    if(args.test){
        play_with_solution("../data/"+std::to_string(ROW)+"x"+std::to_string(COL)+".csv");
    }
    else{
        //DFPNS_test(args);
        PNS_test(args);
    }
    return 0;
}
