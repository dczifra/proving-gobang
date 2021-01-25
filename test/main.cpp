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
#include "logger.h"
#include "node.h"

Args::Args(int argc, char* argv[]){
    int i=0;
    while(i<argc){
        if((std::string) argv[i] == "--log") log = true;
        else if((std::string) argv[i] == "--play") play = true;
        else if((std::string) argv[i] == "--quiet") talky = false;
        else if((std::string) argv[i] == "--test") test = true;
        else if((std::string) argv[i] == "--disproof") disproof = true;
        else if((std::string) argv[i] == "--lines") show_lines = true;
        else if((std::string) argv[i] == "-start"){
            START = std::stoi(argv[++i]);
        }
        else if((std::string) argv[i] == "-potencial_n"){
            potencial_n = std::stoi(argv[++i]);
        }
        else if((std::string) argv[i] == "--PNS2") PNS_square = true;
        else if((std::string )argv[i] == "--help"){
            std::cout<<"Help for AMOBA\nARGS:\n";
            std::cout<<"--play: Play with tree\n";
            std::cout<<"--log: log root PN and DN\n";
            std::cout<<"--test: Tets mode, you can play the solution\n";
        }
        i++;
    }
}

std::string Args::get_filename(){
    std::string folder = (disproof ? "../data/disproof/" : "../data/proof/");
    std::string filename =  std::to_string(ROW)+"x"+std::to_string(COL)+".csv";
    return folder + filename;
}

void PNS_test(Args& args){
    Board b;
    int player = 1;
    Play::choose_problem(b,player, args.disproof, &args);
    if(args.show_lines){
        display(b, true);
    }
    
    PNS tree(&args);
    PNSNode* node = new PNSNode(b, &args);
    std::cout<<"Root node heuristic value: "<<node->heuristic_value<<std::endl;

    tree.init_PN_search(node);
    // === Eval all children first ===
    tree.extend_all(node, false);
    for(int i=node->children.size()-1;i>=0;i--){
        std::cout<<"Child "<<i<<std::endl;
        tree.evaluate_node_with_PNS(node->children[i], args.log, false);
        tree.stats(node->children[i], true);
        PNS::logger->log_node(node->children[i],
                        "../data/final/child_"+std::to_string(i)+".sol");
        //if(i==30) tree.delete_all(node);
    }

    if(args.PNS_square){
        std::cout<<"PNS2"<<std::endl;
        tree.evaluate_node_with_PNS_square(node, args.log, false);
    }
    else{
        tree.evaluate_node_with_PNS(node, args.log, false);
    }
    
    tree.stats(node, true);
    PNS::logger->log_node(node, args.get_filename());

    tree.delete_all(node);
    tree.stats(nullptr, true);
    // tree.component_stats();
    std::cout<<"Nodes visited during search: "<<tree.total_state_size<<std::endl;
}

Heuristic PNS::heuristic;
CanonicalOrder PNS::isom_machine;
Logger* PNS::logger;
Licit PNS::licit;

int main(int argc, char* argv[]){
    Args args(argc, argv);
    std::string spam = (COL > 9 ? "#" : "");
    printf("#############%s\n",spam.c_str());
    printf("# Board %dx%d #\n", ROW, COL);
    printf("#############%s\n", spam.c_str());
    if(args.show_lines) for(auto line : PNS::heuristic.all_linesinfo) display(line.line_board, true);
    
    PNS::logger = new Logger();
    PNS::logger->init(args.disproof);

    if(args.test){
        Play game(args.get_filename(), args.disproof, args.talky, &args);
        game.play_with_solution();
    }
    else{
        //DFPNS_test(args);
        PNS_test(args);
    }
    return 0;
}
