#define DEBUG false
#define TRANSPOSITION_TABLE false
#define RECURSIVE_LINE_SEARCH true
// this macro does not work


#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <dirent.h>

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
    std::string folder = (disproof ? "data/disproof/" : "data/proof/");
    std::string filename =  std::to_string(ROW)+"x"+std::to_string(COL)+".csv";
    return folder + filename;
}

void add_proven_nodes(PNS& tree, std::string folder){
    //std::cout<<"";
    DIR *dir;
    struct dirent *ent;
    if((dir=opendir(folder.c_str())) != NULL){
        while((ent=readdir(dir)) != NULL){
            if(ent->d_name[0] == 'c'){
                std::string filename = folder + "/"+(std::string) ent->d_name;
                Play::read_solution(filename, tree);
                std::cout<<ent->d_name<<" processed\n";
            }
        }
    }
}

void eval_child(Node* node, PNS& tree, Args& args){
    if(!node->extended) tree.extend_all((PNSNode*) node, false);
    for(int i=0; i<node->children.size(); i++){
        std::cout<<"Child "<<i<<std::endl;
        if(!node->children[i]->is_inner()) display(node->children[i]->get_board(), true);
        tree.evaluate_node_with_PNS_square((PNSNode*)node->children[i], args.log, false);
        tree.stats(node->children[i], true);
        PNS::logger->log_node(node->children[i],
                              "data/final/child_"+std::to_string(i)+".sol");
        tree.delete_all(node->children[i]);
    }
}

void eval_all_OR_descendents(Node* node, PNS& tree, Args& args, int depth, PNS& sol){
    if(!node->extended) tree.extend_all((PNSNode*) node, false);
    if(!node->is_inner()){
        if(sol.get_states(node->get_board()) != nullptr){
            return;
        }
        else{
            sol.add_board(node->get_board(), new PNSNode(node->get_board()));
        }
    }

    for(int i=0;i<node->children.size();i++){
        //std::cout<<"I "<<i<<std::endl;
        Node* child = node->children[i];
        if(child==nullptr) assert(0);
        if(child->is_inner() || (child->type == OR && depth < 0)){
            eval_all_OR_descendents(child, tree, args, depth+1, sol);
        }
        else{
            assert(!child->is_inner());
            const Board act_board(child->get_board());
            if(sol.get_states(act_board) == nullptr){
                sol.add_board(act_board, new PNSNode(act_board));
                tree.evaluate_node_with_PNS(child, args.log, false);
                tree.stats(child, true);
                display(act_board, true);
                PNS::logger->log_node(child,
                                    "data/board_sol/"+act_board.to_string()+".sol");
            }
        }

        tree.delete_all(child);
        node->children[i]=nullptr;
    }
}

void PNS_test(Args& args){
    Board b;
    int player = 1;
    Play::choose_problem(b,player, args.disproof, &args);
    if(args.show_lines){
        display(b, true);
    }
    
    PNS tree(&args);
    //add_proven_nodes(tree, "../data/final");
    PNSNode* node = new PNSNode(b, &args);
    std::cout<<"Root node heuristic value: "<<node->heuristic_value<<std::endl;

    tree.init_PN_search(node);
    // === Eval all children first ===
    tree.extend_all(node, false);
    //node = (PNSNode*)node->children[1];
    // ============================================
    //PNS sol(&args);
    //eval_all_OR_descendents(node, tree, args, 0, sol);
    //sol.stats(node, true);
    //std::cout<<"Trick end\n";
    eval_child(node, tree, args);
    return;
    // ============================================
    //node = (PNSNode*)node->children[0]->children[0];
    //display(node->get_board(), true);
    //tree.extend_all(node, false);
    //node = (PNSNode*)node->children[0]->children[1]->children[1];
    //std::cout<<node->is_inner()<<std::endl;
    //display(node->get_board(), true);
    //std::cout<<node->get_board().white<<" "<<node->get_board().black<<" "<<node->get_board().node_type<<" "<<node->get_board().forbidden_all<<"\n";
    
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
board_int Board::base_forbidden = PNS::heuristic.forbidden_all;

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
        //Play game("data/board_sol/36283883716651_4_0_0.sol", args.disproof, args.talky, &args);
        Play game(args.get_filename(), args.disproof, args.talky, &args);
        game.play_with_solution();
    }
    else{
        //DFPNS_test(args);
        PNS_test(args);
    }
    return 0;
}
