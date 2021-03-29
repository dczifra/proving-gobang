#include "logger.h"
#include "counter.h"
#include "common.h"
#include<iostream>
#include<fstream>

void Logger::init(bool disproof){
    #if LOG
    std::string folder = (disproof ? "../logs/disproof_" : "../logs/proof_");
    std::string filename =  std::to_string(ROW)+"x"+std::to_string(COL)+".csv";
    logstream.open(folder + filename, std::ofstream::out | std::ofstream::binary);
    logstream<<"white;black;current_player;pn;dn;empty_cells;potential;node_count;l0;l1;l2;l3;l4;l5;l6;l7"<<std::endl;
    #endif
}

void Logger::log(Node* node, std::ofstream& file){
    if(node == nullptr || node->is_inner()) return;
    else if(logged_states.find(node->get_board()) != logged_states.end()){
        return;
    }
    else{
        logged_states[node->get_board()] = true;
    }

    Counter counter;
    Board act_board(node->get_board());

    file<<act_board.white<<" "<<act_board.black<<" "<<act_board.node_type<<" ";
    file<<node->pn<<" "<<node->dn<<" ";
    file<<act_board.get_valids_num()<<" ";
    file<<((PNSNode*)node)->heuristic_value<<" ";
    file<<counter.count_nodes(node)<<" ";
    file<<act_board.heuristic_layers(PNS::heuristic.all_linesinfo);
    file<<std::endl;
}

void Logger::log_data(Node* node, std::set<Board>& logged, int depth, std::ofstream& file){
    if(node == nullptr) return;
    else if(node->is_inner() || logged.find(node->get_board()) == logged.end()){
        if(!node->is_inner()){
            Board act_board (node->get_board());
            logged.insert(act_board);
            log(node, file);
        }
        
        if(PNS::keep_only_one_child(node)){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = PNS::get_min_children_index(node, proof_type);
            if (min_ind == UINT_MAX) return; // node is a leaf
            else log_data(node->children[min_ind], logged, depth, file);
        }
        else{
            for(int i=0;i<node->child_num;i++){
                log_data(node->children[i], logged, depth+1, file);
            }
        }
    }
}

void Logger::log_solution_min(Node* node, std::ofstream& file, std::string& filebuffer, std::set<Board>& logged, int depth){
    if(node == nullptr) return;
    else if(node->is_inner() || logged.find(node->get_board()) == logged.end()){
        if(!node->is_inner()){
            Board act_board (node->get_board());
            logged.insert(act_board);
            if (filebuffer.length() + 1 >= 100*1024*1024) // 100 MiB
            {
                file<<filebuffer;
                filebuffer.resize(0);
            }
            std::string line((char*) &act_board, sizeof(Board));
            std::string pn((char*) &node->pn, sizeof(int));
            std::string dn((char*) &node->dn, sizeof(int));
            filebuffer.append(line+pn+dn);
            board_int common = PNS::heuristic.forbidden_all;
            if(depth > LOG_CUT_DEPTH){
                return;
            }
        }
        
        if(PNS::keep_only_one_child(node)){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = PNS::get_min_children_index(node, proof_type);
            if (min_ind == UINT_MAX) return; // node is a leaf
            else log_solution_min(node->children[min_ind], file, filebuffer, logged, depth);
        }
        else{
            for(int i=0;i<node->child_num;i++){
                log_solution_min(node->children[i], file, filebuffer, logged, depth+1);
            }
        }
    }
}

void Logger::log_statistics(Node* node, std::string filename){
    std::ofstream file(filename.c_str(), std::ofstream::out);
    file<<"white;black;current_player;pn;dn;empty_cells;potential;node_count;l0;l1;l2;l3;l4;l5;l6;l7"<<std::endl;
    
    std::set<Board> logged;
    log_data(node, logged, 0, file);
    file.close();
}

void Logger::log_node(Node* node, std::string filename){
    std::ofstream file(filename.c_str(), std::ofstream::out | std::ofstream::binary);
    std::string filebuffer;
    filebuffer.reserve(100*1024*1024); // 100 MiB
    std::set<Board> logged;
    log_solution_min(node, file, filebuffer, logged, 0);
    file<<filebuffer;
    file.close();
}

void Logger::log_states(PNS& tree, std::string filename){
    std::ofstream file(filename.c_str(), std::ofstream::out | std::ofstream::binary);
    std::string filebuffer;
    filebuffer.reserve(100*1024*1024); // 100 MiB
    std::set<Board> logged;
    for(auto& p: tree.states){
        log_solution_min(p.second, file, filebuffer, logged, 0);
    }
    file<<filebuffer;
    file.close();
}
