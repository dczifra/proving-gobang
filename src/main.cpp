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
    bool disproof = false;

    Args(int argc, char* argv[]){
        int i=0;
        while(i<argc){
            if((std::string) argv[i] == "--log") log = true;
            else if((std::string) argv[i] == "--play") play = true;
            else if((std::string) argv[i] == "--test") test = true;
            else if((std::string) argv[i] == "--disproof") disproof = true;
            else if((std::string )argv[i] == "--help"){
                std::cout<<"Help for AMOBA\nARGS:\n";
                std::cout<<"--play: Play with tree\n";
                std::cout<<"--log: log root PN and DN\n";
                std::cout<<"--test: Tets mode, you can play the solution\n";
            }
            i++;
        }
    }

    std::string get_filename(){
        std::string folder = (disproof ? "../data/disproof/" : "../data/proof/");
        std::string filename =  std::to_string(ROW)+"x"+std::to_string(COL)+".csv";
        return folder + filename;
    }
};

NodeType choose_problem(Board& b, int& player, Args& args){
    if(args.disproof) b.move({0,1,ROW*COL -1}, player);
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
    while(std::getline (file, s)){
        //std::cout<<s<<std::endl;
        unsigned int pn, dn;
        Board b;

        std::stringstream sstream(s);
        sstream>>b.white>>b.black>>b.node_type>>pn>>dn;
        states[b] = nullptr;
    }
}

void play_with_solution(Args& args){
    // === Read Solution Tree ===
    std::unordered_map<Board, PNS::PNSNode*, Board_Hash> states;
    read_solution(args.get_filename(), states);
    printf("Proof/disproof tree size: %zu\n", states.size());

    // === Init variables ===
    PNS tree;
    Board b;
    int act=-1, human_player, player = 1;
    choose_problem(b,player, args);
    human_player = -player;


    while(!tree.game_ended(b, act)){
        std::vector<int> color;
        // === Human player can choose ===
        act = -1;
        if(player == human_player ){
            int row, col;
            std::cin>>row>>col;
            act = col*ROW+row;
            
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

void PNS_test(Args& args){
    Board b;
    int player = 1;
    choose_problem(b,player, args);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, -1, -1, PNS::heuristic);
    tree.init_PN_search(node);

    tree.evalueate_node_with_PNS(node, args.log);
    tree.stats(node, true);
    
    std::ofstream logfile(args.get_filename());
    tree.log_solution_min(node, logfile);
}

void DFPNS_test(Args& args){
    Board b;
    int player = 1;
    //choose_problem(b,player, args);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, -1, -1, PNS::heuristic);
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

Heuristic PNS::heuristic;

int main(int argc, char* argv[]){
    printf("#############\n");
    printf("# Board %dx%d #\n", ROW, COL);
    printf("#############\n");

    Args args(argc, argv);

    if(args.test){
        play_with_solution(args);
    }
    else{
        //DFPNS_test(args);
        PNS_test(args);
    }
    return 0;
}
