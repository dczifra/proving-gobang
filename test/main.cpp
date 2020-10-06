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
#include "play.h"
#include "canonicalorder.h"

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

void PNS_test(Args& args){
    Board b;
    int player = 1;
    Play::choose_problem(b,player, args.disproof);

    PNS tree;
    PNS::PNSNode* node = new PNS::PNSNode(b, 0, -1, -1, PNS::heuristic);
    tree.init_PN_search(node);

    tree.evalueate_node_with_PNS(node, args.log, false);
    tree.stats(node, true);
    
    std::ofstream logfile(args.get_filename());
    std::set<Board> logged;
    tree.log_solution_min(node, logfile, logged);
    logfile.close();

    tree.delete_all(node);
    tree.stats(nullptr, true);
}

void DFPNS_test(Args& args){
    Board b;
    int player = 1;
    Play::choose_problem(b,player, args.disproof);

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
CanonicalOrder PNS::isom_machine;

int main(int argc, char* argv[]){
    std::string spam = (COL > 9 ? "#" : "");
    printf("#############%s\n",spam.c_str());
    printf("# Board %dx%d #\n", ROW, COL);
    printf("#############%s\n", spam.c_str());

    Args args(argc, argv);

    if(args.test){
        Play game(args.get_filename(), args.disproof);
        game.play_with_solution2();
    }
    else{
        //DFPNS_test(args);
        PNS_test(args);
    }
    return 0;
}
