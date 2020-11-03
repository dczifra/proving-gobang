#include "logger.h"

void Logger::init(bool disproof){
    std::string folder = (disproof ? "../logs/disproof_" : "../logs/proof_");
    std::string filename =  std::to_string(ROW)+"x"+std::to_string(COL)+".csv";
    logstream.open(folder + filename);
    logstream<<"white black current_player pn dn empty_cells potential l0 l1 l2 l3 l4 l5 l6 l7"<<std::endl;
}

void Logger::log(PNS::PNSNode* node, Heuristic& h){
    if(logged_states.find(node->board) != logged_states.end()){
        return;
    }
    else{
        logged_states[node->board] = true;
    }

    logstream<<node->board.white<<" "<<node->board.black<<" "<<node->board.node_type<<" ";
    logstream<<node->pn<<" "<<node->dn<<" ";
    logstream<<__builtin_popcountll(node->board.get_valids())<<" ";
    logstream<<node->heuristic_value<<" ";
    logstream<<node->board.heuristic_layers(h.all_linesinfo);
    logstream<<std::endl;
}

void Logger::log_solution_min(PNS::PNSNode* node, std::ofstream& file, std::set<Board>& logged){
    if(node == nullptr) return;
    else if(logged.find(node->board) == logged.end()){
        logged.insert(node->board);
        file<<node->board.white<<" "<<node->board.black<<" "<<node->board.node_type<<" "<<node->pn<<" "<<node->dn<<std::endl;
        
        if(PNS::keep_only_one_child(node)){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = PNS::get_min_children_index(node, proof_type);
            if (min_ind == UINT_MAX) return;
            else log_solution_min(node->children[min_ind], file, logged);
        }
        else{
            for(int i=0;i<node->child_num;i++){
                log_solution_min(node->children[i], file, logged);
            }
        }
    }
}
