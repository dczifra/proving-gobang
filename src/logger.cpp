#include "logger.h"
#include "counter.h"
#include "common.h"

void Logger::init(bool disproof){
    #if LOG
    std::string folder = (disproof ? "../logs/disproof_" : "../logs/proof_");
    std::string filename =  std::to_string(ROW)+"x"+std::to_string(COL)+".csv";
    logstream.open(folder + filename);
    logstream<<"white black current_player pn dn empty_cells potential node_count l0 l1 l2 l3 l4 l5 l6 l7"<<std::endl;
    #endif
}

void Logger::log(Node* node, Heuristic& h){
    if(node == nullptr || node->is_inner()) return;
    else if(logged_states.find(node->get_board()) != logged_states.end()){
        return;
    }
    else{
        logged_states[node->get_board()] = true;
    }

    Counter counter;
    Board act_board(node->get_board());

    logstream<<act_board.white<<" "<<act_board.black<<" "<<act_board.node_type<<" ";
    logstream<<node->pn<<" "<<node->dn<<" ";
    logstream<<act_board.get_valids_num()<<" ";
    logstream<<((PNSNode*)node)->heuristic_value<<" ";
    logstream<<counter.count_nodes(node)<<" ";
    logstream<<act_board.heuristic_layers(h.all_linesinfo);
    logstream<<std::endl;
}

void Logger::log_solution_min(Node* node, std::ofstream& file, std::set<Board>& logged){
    if(node == nullptr) return;
    else if(node->is_inner() || logged.find(node->get_board()) == logged.end()){
        if(!node->is_inner()){
            Board act_board (node->get_board());
            logged.insert(act_board);
            file<<act_board.white<<" "<<act_board.black<<" "<<act_board.node_type<<" "<<
                act_board.score_left<<" "<<act_board.score_right<<" "<<
                act_board.forbidden_all<<" "<<node->pn<<" "<<node->dn<<std::endl;
        }

        if(PNS::keep_only_one_child(node)){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = PNS::get_min_children_index(node, proof_type);
            if (min_ind == UINT_MAX) return; // node is a leaf
            else log_solution_min(node->children[min_ind], file, logged);
        }
        else{
            for(int i=0;i<node->child_num;i++){
                log_solution_min(node->children[i], file, logged);
            }
        }
    }
}


void Logger::log_node(Node* node, std::string filename){
    std::ofstream file(filename.c_str());
    std::set<Board> logged;
    log_solution_min(node, file, logged);
    file.close();
}
